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
		m_RenderContext.Init(m_Window.Handle(), m_Window.ClientArea());

		m_Window.SetCharKeyCallback(
			'I', 
			[this](bool isReleased) { 
				if (!isReleased && !m_Window.IsWindowed())
					m_Window.Restore();
				else if (m_Window.IsWindowed())
					m_CMLogger.LogInfo(L"CMRenderer [CharKeyCallback ('r')] | Windowed key was pressed, but window is already windowed.\n");
			}
		);

		m_Window.SetCharKeyCallback(
			'O',
			[this](bool isReleased) {
				if (!isReleased && !m_Window.IsMaximized())
					m_Window.Maximize();
				else if (m_Window.IsMaximized())
					m_CMLogger.LogInfo(L"CMRenderer [CharKeyCallback ('O')] | Maximize key was pressed, but window is already maximized.\n");
			}
		);


		m_Window.SetCharKeyCallback(
			'P',
			[this](bool isReleased) {
				if (!isReleased && !m_Window.IsMinimized())
					m_Window.Minimize();
				else if (m_Window.IsMinimized())
					m_CMLogger.LogInfo(L"CMRenderer [CharKeyCallback ('P')] | Minimize key was pressed, but window is already minimized.\n");
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
		std::array<float, 12> array = {
			 -0.5f,  0.5f,
			  0.5f, -0.5f,
			 -0.5f, -0.5f,
			  0.5f,  0.5f,
		};

		std::array<short, 6> indices = {
			0, 1, 2,
			0, 3, 1
		};

		float angle = 0.0f;
		while (m_Window.IsRunning())
		{
			m_Window.HandleMessages();

			// Quit message has been received.
			if (!m_Window.IsRunning())
				break;

			// TODO: Fix memory leak from COM exceptions being thrown from toggling fullscreen state...
			/*if (m_Window.WasFullscreen())
				m_RenderContext.ToggleFullscreen(true);
			else if (m_Window.WasWindowed())
				m_RenderContext.ToggleFullscreen(false);*/

			if (m_Window.IsFocused())
			{
				m_RenderContext.Clear({ 0.5f, 0.0f, 0.5f, 0.0f });
				m_RenderContext.DrawIndexed(array, indices, angle);
				m_RenderContext.Present();
			}

			angle += 0.05f;

			Sleep(10);
		}
	}
#pragma endregion
#pragma region Private
#pragma endregion
}