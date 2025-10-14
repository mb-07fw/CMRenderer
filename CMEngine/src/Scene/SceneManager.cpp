#include "PCH.hpp"
#include "Scene/SceneManager.hpp"

namespace CMEngine::Scene
{
	SceneManager::SceneManager(ECS::ECS& ecs) noexcept
		: m_ECS(ecs)
	{
	}

	[[nodiscard]] SceneID SceneManager::NewScene(const CameraData& mainCamera) noexcept
	{
		m_Scenes.emplace_back(mainCamera);
		
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
				for (const Node::NodePtr<>& pNode : currentScene.RootNode().Nodes())
				{
					ImGuiTreeNodeFlags flags =
						ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
					if (pNode->IsLeaf())
						flags |= ImGuiTreeNodeFlags_Leaf;

					if (ImGui::TreeNodeEx(pNode->Name().c_str(), flags))
					{
						switch (pNode->Type())
						{
						case Node::DerivedType::Invalid: [[fallthrough]];
						default:
							ImGui::TextDisabled("No inspector available");
							break;
						case Node::DerivedType::Camera3D:
						{
							Node::NodePtr<Node::Camera3D> pCamera = std::dynamic_pointer_cast<Node::Camera3D>(pNode);
							ImGui::Text("Camera3D node");
							break;
						}
						}

						ImGui::TreePop();
					}
				}
			}

		ImGui::End();
	}

	void SceneManager::SetActiveScene(SceneID index) noexcept
	{
		if (m_ActiveSceneIndex != S_INVALID_SCENE_INDEX)
			m_Scenes.at(index).OnDeactivate();

		m_ActiveSceneIndex = index;
		m_Scenes.at(index).OnActivate();
	}
}
