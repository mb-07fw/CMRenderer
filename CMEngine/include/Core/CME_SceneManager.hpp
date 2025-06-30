#pragma once

#include "Core/CME_AssetManager.hpp"
#include "Core/CME_Scene.hpp"
#include "CMC_ECS.hpp"

namespace CMEngine
{
	class CMSceneManager
	{
	public:
		CMSceneManager(
			CMCommon::CMLoggerWide& logger,
			CMCommon::CMECS& ecs,
			Asset::CMAssetManager& assetManager,
			DirectXAPI::DX11::DXRenderer& renderer
		) noexcept;
		~CMSceneManager() = default;
	public:
		void UpdateActiveScene(float deltaTime) noexcept;
		void TransitionScene(CMStockSceneType type) noexcept;
	private:
		CMCommon::CMLoggerWide& m_Logger;
		CMCommon::CMECS& m_ECS;
		Asset::CMAssetManager& m_AssetManager;
		DirectXAPI::DX11::DXRenderer& m_Renderer; // Note to self : Stop being lazy and make this a command queue.
		CMTestScene m_TestScene;
		//std::vector<ICMScene*> m_Scenes; // Change this to shared_ptr's or arena allocated scenes.
		ICMScene* mP_ActiveScene = nullptr;
		bool m_StockSceneActive = false;
	};
}