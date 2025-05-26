#include "CME_PCH.hpp"
#include "CME_Engine.hpp"
#include "CMC_Paths.hpp"
#include "CMC_Utility.hpp"
#include "CMC_Macros.hpp"

/*
 *  TODO in order :
 *		1. Add window state rememberance (Minimized -> Previous State)
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
		constexpr float TARGET_FRAME_TIME = 1000.0f / 60.0f;

		float deltaTime = 0.0f;
		while (m_Renderer.Window().IsRunning())
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			m_Renderer.UpdateWindow();

			if (!m_Renderer.Window().IsMinimized())
			{
				m_Renderer.Clear(CMCommon::NormColor{ 0.0f, 0.0f, 0.0f, 1.0f });

				m_SceneManager.UpdateActiveScene(deltaTime);

				ShowEngineControl(deltaTime);

				m_Renderer.Present();
			}

			auto endTime = std::chrono::high_resolution_clock::now();

			std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

			deltaTime = static_cast<float>(duration.count());

			std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(TARGET_FRAME_TIME - deltaTime));
		}
	}

	void CMEngine::ShowEngineControl(float deltaTime) noexcept
	{
		CMRenderer::CMDirectX::DXContext& renderContextRef = m_Renderer.RenderContext();

		m_EngineLogger.LogFatalNLIf(
			!renderContextRef.IsInitialized(),
			L"CMEngine [ShowEngineControl] | Context isn't initialized."
		);

		renderContextRef.ImGuiBegin("Engine");

		ImGui::Text("Millis per frame : %.2f", deltaTime);

		//ImGui::Text("Exclusive fullscreen : %d", renderContextRef.IsFullscreen());

		/*if (renderContextRef.ImGuiButton("Maximize") && !m_Renderer.Window().IsMaximized())
			m_Renderer.Window().Maximize();
		else if (renderContextRef.ImGuiButton("Minimize"))
			m_Renderer.Window().Minimize();*/

		renderContextRef.ImGuiEnd();
	}

	CMEngine::~CMEngine() noexcept
	{
		m_Renderer.Shutdown();
	}
}