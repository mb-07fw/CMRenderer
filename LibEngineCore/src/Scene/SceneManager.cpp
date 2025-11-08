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

	[[nodiscard]] Scene& SceneManager::RetrieveScene(size_t index) noexcept
	{
		return m_Scenes.at(index);
	}

	void SceneManager::DisplaySceneGraph() noexcept
	{
		if (m_ActiveSceneIndex == S_INVALID_SCENE_INDEX)
			return;

		Scene& currentScene = m_Scenes.at(m_ActiveSceneIndex);

		if (ImGui::Begin("Scene"))
			if (ImGui::CollapsingHeader("Graph", ImGuiTreeNodeFlags_DefaultOpen))
				for (const Node::Node& node : currentScene.Graph().Root().Nodes)
				{
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;

					if (Node::IDerivedNode::IsLeaf(node.Type))
						flags |= ImGuiTreeNodeFlags_Leaf;

					std::string_view nodeName = Node::IDerivedNode::NameOf(node.Type);
					if (ImGui::TreeNodeEx(nodeName.data(), flags))
					{
						switch (node.Type)
						{
						case Node::NodeType::Invalid:
						default:
							ImGui::TextDisabled("No inspector available... (ya dun goofed up)");
							break;
						case Node::NodeType::Camera3D:
						{
							View<CameraComponent> cameraComponent = m_CameraSystem.GetCamera(node.Entity);
							CM_ENGINE_ASSERT(cameraComponent.NonNull());

							Node::Camera3D camera3D(node.Entity, *cameraComponent);

							Float3 origin = camera3D.GetOrigin();
							ImGui::SliderFloat3("Origin XYZ", origin.Underlying(), -20.0f, 20.0f);

							camera3D.UpdateOrigin(origin);
							break;
						}
						case Node::NodeType::GameObject:
							if (View<TransformComponent> component = m_ECS.TryGetComponent<TransformComponent>(node.Entity);
								component.NonNull())
								DisplayTransformComponentWidget(*component);
							if (View<MeshComponent> component = m_ECS.TryGetComponent<MeshComponent>(node.Entity);
								component.NonNull())
								DisplayMeshComponentWidget(*component);
							if (View<MaterialComponent> component = m_ECS.TryGetComponent<MaterialComponent>(node.Entity);
								component.NonNull())
								DisplayMaterialComponentWidget(*component);

							break;
						}

						ImGui::TreePop();
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

	void SceneManager::DisplayTransformComponentWidget(TransformComponent& transform) noexcept
	{
		if (!ImGui::TreeNodeEx("TransformComponent", ImGuiTreeNodeFlags_SpanAvailWidth))
			return;

		Transform& transformData = transform.Transform;
		Float3 previousTranslation = transformData.Translation;
		Float3 previousRotation = transformData.Rotation;
		Float3 previousScaling = transformData.Scaling;

		ImGui::SliderFloat3("Translation", transformData.Translation.Underlying(), -20.0f, 20.0f);
		ImGui::SliderFloat3("Rotation", transformData.Rotation.Underlying(), -20.0f, 20.0f);
		ImGui::SliderFloat3("Scaling", transformData.Scaling.Underlying(), -20.0f, 20.0f);

		if (!transformData.Translation.IsNearEqual(previousTranslation) ||
			!transformData.Rotation.IsNearEqual(previousRotation) ||
			!transformData.Scaling.IsNearEqual(previousScaling))
		{
			transform.CreateModelMatrix();
			transform.Dirty = true;
		}

		ImGui::TreePop();
	}

	void SceneManager::DisplayMeshComponentWidget(const MeshComponent& mesh) noexcept
	{
		if (!ImGui::TreeNodeEx("MeshComponent", ImGuiTreeNodeFlags_SpanAvailWidth))
			return;

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.9f, 1.0f), "MeshID: %u", mesh.ID);
		ImGui::TreePop();
	}

	void SceneManager::DisplayMaterialComponentWidget(const MaterialComponent& material) noexcept
	{
		if (!ImGui::TreeNodeEx("MaterialComponent", ImGuiTreeNodeFlags_SpanAvailWidth))
			return;

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.9f, 1.0f), "MaterialID: %u", material.ID);
		ImGui::TreePop();
	}
}
