#pragma once

#include "ECS/ECS.hpp"
#include "Scene/Scene.hpp"
#include "Scene/CameraSystem.hpp"
#include "Platform.hpp"
#include "Component.hpp"

namespace CMEngine::Scene
{
	class SceneManager
	{
	public:
		SceneManager(ECS::ECS& ecs, AWindow& window) noexcept;
		~SceneManager() noexcept;
	public:
		[[nodiscard]] SceneID NewScene() noexcept;
		[[nodiscard]] Scene& RetrieveScene(size_t index) noexcept;
		
		void DisplaySceneGraph() noexcept;

		inline CameraSystem& GetCameraSystem() noexcept { return m_CameraSystem; }
	private:
		void OnWindowResize(Float2 res) noexcept;
		static void OnWindowResizeThunk(Float2 res, void* pThis) noexcept;

		void SetActiveScene(SceneID index) noexcept;

		void DisplayTransformComponentWidget(TransformComponent& transform, uint32_t nodeIndex) noexcept;
		void DisplayMeshComponentWidget(const MeshComponent& mesh, uint32_t nodeIndex) noexcept;
		void DisplayMaterialComponentWidget(const MaterialComponent& material, uint32_t nodeIndex) noexcept;
	private:
		static constexpr size_t S_INVALID_SCENE_INDEX = ~static_cast<size_t>(0ull);
		ECS::ECS& m_ECS;
		AWindow& m_Window;
		CameraSystem m_CameraSystem;
		std::vector<Scene> m_Scenes;
		size_t m_ActiveSceneIndex = S_INVALID_SCENE_INDEX;
	};
}