#include "Core/PCH.hpp"
#include "Core/SceneManager.hpp"

namespace CMEngine::Core
{
	SceneManager::SceneManager(
		Common::LoggerWide& logger,
		ECS& ecs,
		Asset::AssetManager& assetManager,
		DX::DX11::Renderer& renderer
	) noexcept
		: m_Logger(logger),
		  m_ECS(ecs),
		  m_AssetManager(assetManager),
		  m_Renderer(renderer),
		  m_TestScene(m_Logger, m_ECS, m_AssetManager, m_Renderer)
	{
		//m_Scenes.emplace_back(&m_TestScene);
	}

	void SceneManager::UpdateActiveScene(float deltaTime) noexcept
	{
		m_Logger.LogFatalNLIf(
			mP_ActiveScene == nullptr,
			L"SceneManager [UpdateActiveScene] | Active scene is nullptr."
		);

		mP_ActiveScene->OnUpdate(deltaTime);
	}

	void SceneManager::TransitionScene(StockSceneType type) noexcept
	{
		if (mP_ActiveScene != nullptr)
			mP_ActiveScene->OnEnd();

		switch (type)
		{
		case StockSceneType::TEST_SCENE:
			mP_ActiveScene = &m_TestScene;
			break;
		}

		mP_ActiveScene->OnStart();

		m_StockSceneActive = true;
	}
}