#include "CME_PCH.hpp"
#include "CME_Engine.hpp"
#include "CMC_Paths.hpp"
#include "CMC_Utility.hpp"
#include "CMC_Macros.hpp"
#include "CMC_Shapes.hpp"

/*
 *  TODO in order :
 *		1. Introduce a callback for window resizing so classes outside of CMRenderer don't have to manually set it for DXContext.
 *	Future :
 *		Add instanced renderering, vertex array specialization, etc.
 */

namespace CMEngine
{
	CMEngine::CMEngine() noexcept
		: m_Renderer(CMRenderer::CMWindowData(L"Title", false, 0, 0)),
		  m_SceneManager(m_EngineLogger, m_ECS,  m_Renderer)
	{
  		m_EngineLogger.OpenFileInDirectory(S_LIFETIME_LOG_FILE_NAME, L"./logs/");

		if (!m_EngineLogger.IsStreamOpen())
			CM_BREAK_DEBUGGER();

		m_Renderer.Init();
		m_SceneManager.TransitionScene(CMStockSceneType::TEST_SCENE);

		m_EngineLogger.LogInfoNL(L"CMEngine [()] | Constructed.");
	}

	void CMEngine::Run() noexcept
	{
		CMCommon::CMRect rect = { { 0.0f, 0.0f } };

		float deltaTime = 0.0f;
		while (m_Renderer.Window().IsRunning())
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			m_Renderer.UpdateWindow();

			m_Renderer.Clear(CMCommon::NormColor{ 0.0f, 0.0f, 0.0f, 1.0f });

			m_SceneManager.UpdateActiveScene(deltaTime);

			m_Renderer.Present();

			auto endTime = std::chrono::high_resolution_clock::now();

			deltaTime = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count());
		}
	}

	CMEngine::~CMEngine() noexcept
	{
		m_Renderer.Shutdown();
	}
}