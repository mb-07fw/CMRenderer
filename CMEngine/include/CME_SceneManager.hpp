#pragma once

#include "CMC_ECS.hpp"
#include "CME_AssetManager.hpp"
#include "CME_Scene.hpp"

namespace CMEngine
{
	class CMSceneManager
	{
	public:
		CMSceneManager(
			CMCommon::CMLoggerWide& logger,
			CMCommon::CMECS& ecs,
			Asset::CMAssetManager& assetManager,
			CMRenderer::CMRenderer& renderer
		) noexcept;
		~CMSceneManager() = default;
	public:
		void UpdateActiveScene(float deltaTime) noexcept;
		void TransitionScene(CMStockSceneType type) noexcept;
	private:
		CMCommon::CMLoggerWide& m_Logger;
		CMCommon::CMECS& m_ECS;
		Asset::CMAssetManager& m_AssetManager;
		CMRenderer::CMRenderer& m_Renderer; // Note to self : Stop being lazy and make this a command queue.
		CMTestScene m_TestScene;
		//std::vector<ICMScene*> m_Scenes; // Change this to shared_ptr's or arena allocated scenes.
		ICMScene* mP_ActiveScene = nullptr;
		bool m_StockSceneActive = false;
	};
}