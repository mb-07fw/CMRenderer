#pragma once

#include "Core/AssetManager.hpp"
#include "Core/Scene.hpp"
#include "Core/ECS.hpp"

namespace CMEngine::Core
{
	class SceneManager
	{
	public:
		SceneManager(
			Common::LoggerWide& logger,
			ECS& ecs,
			Asset::AssetManager& assetManager,
			DX::DX11::Renderer& renderer
		) noexcept;

		~SceneManager() = default;
	public:
		void UpdateActiveScene(float deltaTime) noexcept;
		void TransitionScene(StockSceneType type) noexcept;
	private:
		Common::LoggerWide& m_Logger;
		ECS& m_ECS;
		Asset::AssetManager& m_AssetManager;
		DX::DX11::Renderer& m_Renderer; // Note to self : Stop being lazy and make this a command queue.
		TestScene m_TestScene;
		//std::vector<ICMScene*> m_Scenes; // Change this to shared_ptr's or arena allocated scenes.
		IScene* mP_ActiveScene = nullptr;
		bool m_StockSceneActive = false;
	};
}