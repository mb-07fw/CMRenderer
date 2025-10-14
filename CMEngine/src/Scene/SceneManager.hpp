#pragma once

#include "ECS/ECS.hpp"
#include "Scene/Scene.hpp"

namespace CMEngine::Scene
{
	class SceneManager
	{
	public:
		SceneManager(ECS::ECS& ecs) noexcept;
		~SceneManager() = default;
	public:
		[[nodiscard]] SceneID NewScene(const CameraData& mainCamera) noexcept;

		void DisplaySceneGraph() noexcept;

		inline Scene& RetrieveScene(size_t index) noexcept { return m_Scenes.at(index); }
	private:
		void SetActiveScene(SceneID index) noexcept;
	private:
		static constexpr size_t S_INVALID_SCENE_INDEX = ~static_cast<size_t>(0ull);
		ECS::ECS& m_ECS;
		size_t m_ActiveSceneIndex = S_INVALID_SCENE_INDEX;
		std::vector<Scene> m_Scenes;
	};
}