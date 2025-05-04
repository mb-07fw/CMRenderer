#include "CME_PCH.hpp"
#include "CME_SceneManager.hpp"

namespace CMEngine
{
	CMSceneManager::CMSceneManager(
		CMCommon::CMLoggerWide& engineLifetimeLoggerRef,
		CMCommon::CMECS& cmECSRef,
		CMRenderer::CMRenderer& rendererRef
	) noexcept
		: m_EngineLoggerRef(engineLifetimeLoggerRef),
		  m_ECSRef(cmECSRef),
		  m_RendererRef(rendererRef),
		  m_TestScene(m_EngineLoggerRef, m_ECSRef, m_RendererRef)
	{
		//m_Scenes.emplace_back(&m_TestScene);
	}

	void CMSceneManager::UpdateActiveScene(float deltaTime) noexcept
	{
		m_EngineLoggerRef.LogFatalNLIf(
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