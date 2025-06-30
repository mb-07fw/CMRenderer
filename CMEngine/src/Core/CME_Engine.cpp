#include "Core/CME_PCH.hpp"
#include "Core/CME_Engine.hpp"
#include "Core/CME_EditorLayer.hpp"
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
		m_Renderer(m_EngineLogger, m_WindowSettings.Current),
		  m_AssetManager(m_EngineLogger),
		  m_SceneManager(m_EngineLogger, m_ECS,  m_AssetManager, m_Renderer),
		  m_LayerStack(m_EngineLogger),
		  m_EngineHeap(m_EngineLogger)
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

		m_EngineLogger.LogInfoNL(L"CMEngine [()] | Constructed.");
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

		CMLayerHandle editorHandle = m_LayerStack.EmplaceBack<CMEditorLayer>(
			m_EngineLogger,
			m_Renderer,
			m_ECS,
			m_AssetManager,
			m_SceneManager,
			m_EngineHeap
		);

		/*std::shared_ptr<CMEditorLayer> pEditorLayer = m_LayerStack.RetrieveAs<CMEditorLayer>(editorHandle);

		m_EngineLogger.LogFatalNLTaggedIf(
			pEditorLayer.get() == nullptr,
			FuncTag,
			L"Failed to retrieve editor layer."
		);*/

		float deltaTime = 0.0f;
		while (m_Window.IsRunning())
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			UpdateWindow();

			if (!m_Window.IsMinimized())
			{
				Clear(CMCommon::NormColor{ 0.0f, 0.0f, 0.0f, 1.0f });

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
			!m_Renderer.IsInitialized(),
			L"CMEngine [ShowEngineControl] | Context isn't initialized."
		);

		m_Renderer.ImGuiBegin("Engine");

		ImGui::Text("Millis per frame : %.2f", deltaTime);

		m_Renderer.ImGuiEnd();
	}

	void CMEngine::UpdateWindow() noexcept
	{
		m_Window.HandleMessages();
	}

	void CMEngine::Clear(CMCommon::NormColor normalizedColor) noexcept
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