#include "PCH.hpp"
#include "Scene/SceneManager.hpp"

namespace CMEngine::Scene
{
	inline std::string ImGuiLabel(const char* base, uint32_t id)
	{
		std::string label = base;
		label.append("##").append(std::to_string(id));
		return label;
	}

	SceneManager::SceneManager(ECS::ECS& ecs, AWindow& window) noexcept
		: m_ECS(ecs),
		  m_Window(window),
		  m_CameraSystem(ecs)
	{
		m_Window.SetCallbackOnResize(OnWindowResizeThunk, this);
	}

	SceneManager::~SceneManager() noexcept
	{
		m_Window.RemoveCallbackOnResize(OnWindowResizeThunk, this);
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
		std::string nodeName;
		uint32_t currentNodeIndex = 0;

		if (ImGui::Begin("Scene"))
			if (ImGui::CollapsingHeader("Graph", ImGuiTreeNodeFlags_DefaultOpen))
				for (const Node::Node& node : currentScene.Graph().Root().Nodes)
				{
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;

					if (Node::IDerivedNode::IsLeaf(node.Type))
						flags |= ImGuiTreeNodeFlags_Leaf;

					nodeName = Node::IDerivedNode::NameOf(node.Type);

					if (nodeName.size() + 3 < nodeName.capacity())
						nodeName.reserve(nodeName.size() + 3);

					/* ImGui shenanigans to make sure widget ID's aren't duplicated. */
					nodeName.append("##").append(std::to_string(currentNodeIndex));

					if (ImGui::TreeNodeEx(nodeName.c_str(), flags))
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
							if (auto component = m_ECS.TryGetComponent<TransformComponent>(node.Entity);
								component.NonNull())
								DisplayTransformComponentWidget(*component, currentNodeIndex);
							if (auto component = m_ECS.TryGetComponent<MeshComponent>(node.Entity);
								component.NonNull())
								DisplayMeshComponentWidget(*component, currentNodeIndex);
							if (auto component = m_ECS.TryGetComponent<MaterialComponent>(node.Entity);
								component.NonNull())
								DisplayMaterialComponentWidget(*component, currentNodeIndex);
							if (auto component = m_ECS.TryGetComponent<TextureComponent>(node.Entity);
								component.NonNull())
								DisplayTextureComponentWidget(*component, currentNodeIndex);

							break;
						}

						ImGui::TreePop();
					}

					currentNodeIndex++;
				}

		ImGui::End();
	}

	void SceneManager::OnWindowResize(Float2 res) noexcept
	{
		/* TODO: Move to CameraSystem... */

		auto cameras = m_ECS.GetSparseSet<CameraComponent>();

		for (auto& camera : cameras->Data())
			camera.SetAspect(res.Aspect());
	}

	void SceneManager::OnWindowResizeThunk(Float2 res, void* pThis) noexcept
	{
		reinterpret_cast<SceneManager*>(pThis)->OnWindowResize(res);
	}

	void SceneManager::SetActiveScene(SceneID index) noexcept
	{
		if (m_ActiveSceneIndex != S_INVALID_SCENE_INDEX)
			m_Scenes.at(index).OnDeactivate();

		m_ActiveSceneIndex = index;
		m_Scenes.at(index).OnActivate();
	}

	void SceneManager::DisplayTransformComponentWidget(TransformComponent& transform, uint32_t nodeIndex) noexcept
	{
		if (!ImGui::TreeNodeEx("TransformComponent", ImGuiTreeNodeFlags_SpanAvailWidth))
			return;

		Transform& transformData = transform.Transform;
		Float3 previousTranslation = transformData.Translation;
		Float3 previousRotation = transformData.Rotation;
		Float3 previousScaling = transformData.Scaling;

		ImGui::SliderFloat3(ImGuiLabel("Translation", nodeIndex).c_str(), transformData.Translation.Underlying(), -50.0f, 50.0f);
		ImGui::SliderFloat3(ImGuiLabel("Rotation", nodeIndex).c_str(), transformData.Rotation.Underlying(), -360.0f, 360.0f);
		ImGui::SliderFloat3(ImGuiLabel("Scaling", nodeIndex).c_str(), transformData.Scaling.Underlying(), -50.0f, 50.0f);

		if (!transformData.Translation.IsNearEqual(previousTranslation) ||
			!transformData.Rotation.IsNearEqual(previousRotation) ||
			!transformData.Scaling.IsNearEqual(previousScaling))
		{
			transform.CreateModelMatrix();
			transform.Dirty = false;
		}

		ImGui::TreePop();
	}

	void SceneManager::DisplayMeshComponentWidget(const MeshComponent& mesh, uint32_t nodeIndex) noexcept
	{
		if (!ImGui::TreeNodeEx(ImGuiLabel("MeshComponent", nodeIndex).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
			return;

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.9f, 1.0f), "MeshID: %u", mesh.ID);
		ImGui::TreePop();
	}

	void SceneManager::DisplayMaterialComponentWidget(const MaterialComponent& material, uint32_t nodeIndex) noexcept
	{
		if (!ImGui::TreeNodeEx(ImGuiLabel("MaterialComponent", nodeIndex).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
			return;

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.9f, 1.0f), "MaterialID: %u", material.ID);
		ImGui::TreePop();
	}

	void SceneManager::DisplayTextureComponentWidget(const TextureComponent& texture, uint32_t nodeIndex) noexcept
	{
		if (!ImGui::TreeNodeEx(ImGuiLabel("TextureComponent", nodeIndex).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
			return;

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.9f, 1.0f), "TextureID: %u", texture.ID);
		ImGui::TreePop();
	}
}
