#include "PCH.hpp"
#include "Scene/SceneManager.hpp"

namespace CMEngine::Scene
{
	SceneManager::SceneManager(ECS::ECS& ecs) noexcept
		: m_ECS(ecs),
		  m_CameraSystem(ecs)
	{
	}

	[[nodiscard]] SceneID SceneManager::NewScene() noexcept
	{
		m_Scenes.emplace_back();
		
		size_t index = m_Scenes.size() - 1;
		SetActiveScene(index);

		return index;
	}

	void SceneManager::DisplaySceneGraph() noexcept
	{
		if (m_ActiveSceneIndex == S_INVALID_SCENE_INDEX)
			return;

		Scene& currentScene = m_Scenes.at(m_ActiveSceneIndex);

		if (ImGui::Begin("Scene"))
			if (ImGui::CollapsingHeader("Graph", ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (const Node::Node& node : currentScene.Graph().Root().Nodes)
				{
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | 
						ImGuiTreeNodeFlags_SpanAvailWidth;

					if (Node::IDerivedNode::IsLeaf(node.Type))
						flags |= ImGuiTreeNodeFlags_Leaf;

					std::string_view nodeName = Node::IDerivedNode::NameOf(node.Type);
					if (ImGui::TreeNodeEx(nodeName.data(), flags))
					{
						switch (node.Type)
						{
						case Node::NodeType::Invalid:
						default:
							ImGui::TextDisabled("No inspector available");
							break;
						case Node::NodeType::Camera3D:
						{
							CameraComponent* pCameraComponent = m_CameraSystem.GetCamera(node.Entity);
							CM_ENGINE_ASSERT(pCameraComponent != nullptr);

							Node::Camera3D camera3D(node.Entity, *pCameraComponent);

							Float3 origin = camera3D.GetOrigin();
							ImGui::SliderFloat3("XYZ", origin.Underlying(), -20.0f, 20.0f);
							camera3D.UpdateOrigin(origin);

							break;
						}
						}

						ImGui::TreePop();
					}
				}

				//for (const Node::NodePtr<>& pNode : currentScene.RootNode().Nodes())
				//{
				//	ImGuiTreeNodeFlags flags =
				//		ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				//	if (pNode->IsLeaf())
				//		flags |= ImGuiTreeNodeFlags_Leaf;

				//	if (ImGui::TreeNodeEx(pNode->Name().c_str(), flags))
				//	{
				//		switch (pNode->Type())
				//		{
				//		case Node::DerivedType::Invalid: [[fallthrough]];
				//		default:
				//			ImGui::TextDisabled("No inspector available");
				//			break;
				//		case Node::DerivedType::Camera3D:
				//		{
				//			Node::Camera3D* pCamera = static_cast<Node::Camera3D*>(pNode.get());
				//			CameraData& data = pCamera->Component().Data;

				//			Float3 previousPos = data.Origin;
				//			ImGui::SliderFloat3("XYZ", data.Origin.Underlying(), -20.0f, 20.0f);

				//			/* Recreate matrices if camera origin changes... */
				//			if (!data.Origin.IsNearEqual(previousPos))
				//				pCamera->Component().CreateMatrices();

				//			break;
				//		}
				//		}

				//		ImGui::TreePop();
				//	}
				//}
			}

		ImGui::End();
	}

	[[nodiscard]] Scene& SceneManager::RetrieveScene(size_t index) noexcept
	{
		return m_Scenes.at(index);
	}

	void SceneManager::SetActiveScene(SceneID index) noexcept
	{
		if (m_ActiveSceneIndex != S_INVALID_SCENE_INDEX)
			m_Scenes.at(index).OnDeactivate();

		m_ActiveSceneIndex = index;
		m_Scenes.at(index).OnActivate();
	}
}
