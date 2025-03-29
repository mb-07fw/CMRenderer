#include "Core/CMPCH.hpp"
#include "CMRenderer.hpp"

namespace CMRenderer
{
	CMRenderer::CMRenderer(CMRendererSettings settings) noexcept
		: m_Settings(settings), m_CMLogger(S_LIFETIME_LOG_FILE_NAME),
		  m_Window(m_Settings, m_CMLogger),
		  m_RenderContext(m_CMLogger)
	{
		/*HRESULT hResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		if (hResult != S_OK)
		{
			m_CMLogger.LogFatal(L"CMRenderer[()] | Failed to initialize COM.\n");
			return;
		}*/

		m_CMLogger.LogInfo(L"CMRenderer [()] | Constructed.\n");
	}

	CMRenderer::~CMRenderer() noexcept
	{
		if (!m_Shutdown)
			Shutdown();

		//CoUninitialize();

		m_CMLogger.LogInfo(L"CMRenderer [~()] | Destroyed.\n");
	}

#pragma region Public
	void CMRenderer::Init() noexcept
	{
		if (m_Initialized)
		{
			m_CMLogger.LogWarning(L"CMRenderer [Init] | Attempted to initialize after initialization has already occured previously.\n");
			return;
		}

		m_Window.Init();
		m_RenderContext.Init(m_Window.Handle(), m_Window.ClientArea(), m_Settings.WindowSettings.Current.UseFullscreen);

		m_Window.SetCharKeyCallback(
			'R', 
			[this](bool isReleased) { 
				if (!isReleased && m_Window.IsFullscreen())
				{
					m_RenderContext.ToggleFullscreen(false); 
					m_Window.Restore();
				}
			}
		);

		m_Initialized = true;
		m_Shutdown = false;
	}

	void CMRenderer::Shutdown() noexcept
	{
		if (m_Shutdown)
		{
			m_CMLogger.LogWarning(L"CMRenderer [Shutdown] | Attempted to shutdown after shutdown has already occured previously.\n");
			return;
		}

		m_Window.Shutdown();
		m_RenderContext.Shutdown();

		m_Initialized = false;
		m_Shutdown = true;
	}

	void CMRenderer::Run() noexcept
	{
		std::array<float, 6> array = {
			  0.0,   0.5f,
			  0.5f, -0.5f,
			 -0.5f, -0.5f
		};

		while (m_Window.IsRunning())
		{
			m_Window.HandleMessages();

			// Quit message has been received.
			if (!m_Window.IsRunning())
				break;

			if (m_Window.WasFullscreen())
				m_RenderContext.ToggleFullscreen(true);
			else if (m_Window.WasResized())
				m_RenderContext.ToggleFullscreen(false);

			m_RenderContext.Clear({ 0.5f, 0.0f, 0.5f, 0.0f });
			m_RenderContext.Draw(array, 3);
			m_RenderContext.Present();

			Sleep(10);
		}
	}
#pragma endregion
#pragma region Private
	void CMRenderer::Suspend() noexcept
	{
		if (m_Settings.WindowSettings.Current.UseFullscreen)
		{
			m_RenderContext.ToggleFullscreen(false);
			m_Window.Minimize();
		}

		m_Window.WaitForMessages(20);

		/*if (m_Window.HasResized())
			m_RenderContext.ResizeTo(m_Window.ClientArea());*/

		if (m_Settings.WindowSettings.Current.UseFullscreen)
		{
			m_RenderContext.ToggleFullscreen(true);
			m_Window.Maximize();
		}
	}


#pragma endregion
}