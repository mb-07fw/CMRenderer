#include "Core/CME_PCH.hpp"
#include "Core/CME_SceneManager.hpp"

namespace CMEngine
{
	CMSceneManager::CMSceneManager(
		CMCommon::CMLoggerWide& logger,
		CMCommon::CMECS& ecs,
		Asset::CMAssetManager& assetManager,
		DirectXAPI::DX11::DXContext& context
	) noexcept
		: m_Logger(logger),
		  m_ECS(ecs),
		  m_AssetManager(assetManager),
		  m_Context(context),
		  m_TestScene(m_Logger, m_ECS, m_AssetManager, m_Context)
	{
		//m_Scenes.emplace_back(&m_TestScene);
	}

	void CMSceneManager::UpdateActiveScene(float deltaTime) noexcept
	{
		m_Logger.LogFatalNLIf(
			mP_ActiveScene == nullptr,
			L"CMSceneManager [UpdateActiveScene] | Active scene is nullptr."
		);

		mP_ActiveScene->OnUpdate(deltaTime);
	}

	void CMSceneManager::TransitionScene(CMStockSceneType type) noexcept
	{
		if (mP_ActiveScene != nullptr)
			mP_ActiveScene->OnEnd();

		switch (type)
		{
		case CMStockSceneType::TEST_SCENE:
			mP_ActiveScene = &m_TestScene;
			break;
		}

		mP_ActiveScene->OnStart();

		m_StockSceneActive = true;
	}
}