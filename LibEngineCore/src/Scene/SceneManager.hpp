#pragma once

#include "ECS/ECS.hpp"
#include "Scene/Scene.hpp"
#include "Scene/CameraSystem.hpp"
#include "Component.hpp"

namespace CMEngine::Scene
{
	class SceneManager
	{
	public:
		SceneManager(ECS::ECS& ecs) noexcept;
		~SceneManager() = default;
	public:
		[[nodiscard]] SceneID NewScene() noexcept;
		[[nodiscard]] Scene& RetrieveScene(size_t index) noexcept;
		
		void DisplaySceneGraph() noexcept;

		inline CameraSystem& GetCameraSystem() noexcept { return m_CameraSystem; }
	private:
		void SetActiveScene(SceneID index) noexcept;

		void DisplayTransformComponentWidget(TransformComponent& transform) noexcept;
		void DisplayMeshComponentWidget(const MeshComponent& mesh) noexcept;
		void DisplayMaterialComponentWidget(const MaterialComponent& material) noexcept;
	private:
		static constexpr size_t S_INVALID_SCENE_INDEX = ~static_cast<size_t>(0ull);
		ECS::ECS& m_ECS;
		CameraSystem m_CameraSystem;
		std::vector<Scene> m_Scenes;
		size_t m_ActiveSceneIndex = S_INVALID_SCENE_INDEX;
	};
}