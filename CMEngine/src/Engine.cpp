#include "Core/PCH.hpp"
#include "Engine.hpp"
#include "Core/EditorLayer.hpp"
#include "Core/Paths.hpp"
#include "Common/Utility.hpp"
#include "Common/Macros.hpp"

namespace CMEngine
{
	CMEngine::CMEngine() noexcept
		: m_EngineHeap(m_EngineLogger),
		  m_WindowSettings(Win::WindowData(L"Title", false, 800, 600)),
		  m_Window(
			m_EngineLogger,
			m_WindowSettings,
			[this]() {
				this->OnWindowResize();
			}
		  ),
		  m_Renderer(m_EngineLogger, m_WindowSettings.Current),
		  m_AssetManager(m_EngineLogger),
		  m_SceneManager(m_EngineLogger, m_ECS,  m_AssetManager, m_Renderer),
		  m_LayerStack(m_EngineLogger)
	{
		constexpr std::wstring_view FuncTag = L"CMEngine [()] | ";

  		m_EngineLogger.OpenFileInDirectory(S_LIFETIME_LOG_FILE_NAME, L"./logs/");

		m_EngineLogger.LogFatalNLTaggedIf(
			!m_EngineLogger.IsStreamOpen(),
			FuncTag,
			L"Failed to open logger's file stream."
		);

		/* For Microsoft DirectX applications, you must initialize the initial
		 *	thread by using Windows::Foundation::Initialize(RO_INIT_MULTITHREADED).
		 */
		m_EngineLogger.LogFatalNLTaggedIf(
			FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED)),
			FuncTag,
			L"Failed to call Windows::Foundation::Initialize."
		);

		m_EngineLogger.LogInfoNLTagged(FuncTag, L"Constructed.");
	}

	CMEngine::~CMEngine() noexcept
	{
		Windows::Foundation::Uninitialize();

		if (m_Initialized)
			Shutdown();

		m_EngineLogger.LogInfoNL(L"CMRenderer [~()] | Destroyed.");
	}

	void CMEngine::Init() noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMEngine [Init] | ";

		m_EngineLogger.LogFatalNLTaggedIf(m_Initialized, FuncTag, L"Engine is already initialized.");

		m_Window.Init();
		m_Renderer.Init(m_Window.Handle());
		m_AssetManager.Init();
		//m_SceneManager.TransitionScene(CMStockSceneType::TEST_SCENE);

		m_Initialized = true;
		m_Shutdown = false;
	}

	void CMEngine::Shutdown() noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMEngine [Shutdown] | ";

		m_EngineLogger.LogFatalNLTaggedIf(m_Shutdown, FuncTag, L"Engine is already shutdown.");

		m_Window.Shutdown();
		m_Renderer.Shutdown();
		m_AssetManager.Shutdown();

		m_LayerStack.Clear();

		m_Initialized = false;
		m_Shutdown = true;
	}
	
	void CMEngine::Run() noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMEngine [Run] | ";
		constexpr float TARGET_FRAME_TIME = 1000.0f / 60.0f;

		m_EngineLogger.LogFatalNLTaggedIf(!m_Initialized, FuncTag, L"Engine is not initialized.");

		Core::LayerHandle editorHandle = m_LayerStack.EmplaceBack<Core::EditorLayer>(
			m_EngineLogger,
			m_Renderer,
			m_Window,
			m_ECS,
			m_AssetManager,
			m_SceneManager,
			m_EngineHeap
		);

		float deltaTime = 0.0f;
		while (m_Window.IsRunning())
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			UpdateWindow();

			if (!m_Window.IsMinimized())
			{
				Clear(Common::NormColor{ 0.0f, 0.0f, 0.0f, 1.0f });

				//m_SceneManager.UpdateActiveScene(deltaTime);
				m_LayerStack.Update(deltaTime);

				ShowEngineControl(deltaTime);

				Present();
			}

			auto endTime = std::chrono::high_resolution_clock::now();

			std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

			deltaTime = static_cast<float>(duration.count());

			std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(TARGET_FRAME_TIME - deltaTime));
		}
	}

	void CMEngine::ShowEngineControl(float deltaTime) noexcept
	{
		m_EngineLogger.LogFatalNLIf(
			!m_Renderer.IsInitialized(),
			L"CMEngine [ShowEngineControl] | Context isn't initialized."
		);

		m_Renderer.ImGuiBegin("Engine");

		m_Renderer.ImGuiText("FPS: %.2f", 1000 / deltaTime);
		m_Renderer.ImGuiText("Millis per frame: %.2f", deltaTime);
		m_Renderer.ImGuiText("Borderless fullscreen: %s", Common::Utility::BoolToString(m_Window.IsMaximized()));
		
		Common::Float2 currentResolution = m_Renderer.CurrentResolution();

		m_Renderer.ImGuiText("Current Resolution: %.0f x %.0f", currentResolution.x, currentResolution.y);

		if (m_Renderer.ImGuiButton("Restore"))
			m_Window.Restore();

		m_Renderer.ImGuiEnd();
	}

	void CMEngine::UpdateWindow() noexcept
	{
		m_Window.HandleMessages();
	}

	void CMEngine::Clear(Common::NormColor normalizedColor) noexcept
	{
		m_Renderer.Clear(normalizedColor);
	}

	void CMEngine::Present() noexcept
	{
		m_Renderer.Present();
	}

	void CMEngine::OnWindowResize() noexcept
	{
		m_Renderer.OnWindowResize();
	}
}