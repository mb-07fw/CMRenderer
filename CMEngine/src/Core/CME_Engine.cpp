#include "Core/CME_PCH.hpp"
#include "Core/CME_Engine.hpp"
#include "Core/CME_Paths.hpp"
#include "CMC_Utility.hpp"
#include "CMC_Macros.hpp"

namespace CMEngine
{
	CMEngine::CMEngine() noexcept
		: m_WindowSettings(CMWindowData(L"Title", false, 800, 600)),
		   m_Window(
			m_EngineLogger,
			m_WindowSettings,
			[this]() {
				this->OnWindowResize();
			}
		  ),
		  m_Context(m_EngineLogger, m_WindowSettings.Current),
		  m_AssetManager(m_EngineLogger),
		  m_SceneManager(m_EngineLogger, m_ECS,  m_AssetManager, m_Context)
	{
  		m_EngineLogger.OpenFileInDirectory(S_LIFETIME_LOG_FILE_NAME, L"./logs/");

		m_EngineLogger.LogFatalNLIf(
			!m_EngineLogger.IsStreamOpen(),
			L"CMEngine [()] | Failed to open logger's file stream."
		);

		/*
		 *	For Microsoft DirectX applications, you must initialize the initial
		 *	thread by using Windows::Foundation::Initialize(RO_INIT_MULTITHREADED).
		 */
		m_EngineLogger.LogFatalNLIf(
			FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED)),
			L"CMEngine [()] | Failed to call Windows::Foundation::Initialize."
		);

		m_Window.Init();
		m_Context.Init(m_Window.Handle());
		m_AssetManager.Init();
		m_SceneManager.TransitionScene(CMStockSceneType::TEST_SCENE);

		m_EngineLogger.LogInfoNL(L"CMEngine [()] | Constructed.");
	}

	CMEngine::~CMEngine() noexcept
	{
		m_Window.Shutdown();
		m_Context.Shutdown();
		m_AssetManager.Shutdown();

		Windows::Foundation::Uninitialize();

		m_EngineLogger.LogInfoNL(L"CMRenderer [~()] | Destroyed.");
	}
	
	void CMEngine::Run() noexcept
	{

	}

	/*void CMEngine::Run() noexcept
	{
		constexpr float TARGET_FRAME_TIME = 1000.0f / 60.0f;

		float deltaTime = 0.0f;
		while (m_Window.IsRunning())
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			UpdateWindow();

			if (!m_Window.IsMinimized())
			{
				Clear(CMCommon::NormColor{ 0.0f, 0.0f, 0.0f, 1.0f });

				m_SceneManager.UpdateActiveScene(deltaTime);

				ShowEngineControl(deltaTime);

				Present();
			}

			auto endTime = std::chrono::high_resolution_clock::now();

			std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

			deltaTime = static_cast<float>(duration.count());

			std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(TARGET_FRAME_TIME - deltaTime));
		}
	}*/

	void CMEngine::ShowEngineControl(float deltaTime) noexcept
	{
		m_EngineLogger.LogFatalNLIf(
			!m_Context.IsInitialized(),
			L"CMEngine [ShowEngineControl] | Context isn't initialized."
		);

		m_Context.ImGuiBegin("Engine");

		ImGui::Text("Millis per frame : %.2f", deltaTime);

		m_Context.ImGuiEnd();
	}

	void CMEngine::UpdateWindow() noexcept
	{
		m_Window.HandleMessages();
	}

	void CMEngine::Clear(CMCommon::NormColor normalizedColor) noexcept
	{
		m_Context.Clear(normalizedColor);
	}

	void CMEngine::Present() noexcept
	{
		m_Context.Present();
	}

	void CMEngine::OnWindowResize() noexcept
	{
		m_Context.OnWindowResize();
	}
}