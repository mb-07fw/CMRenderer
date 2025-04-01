#include "Core/CMPCH.hpp"
#include "CMRenderer.hpp"

namespace CMRenderer
{
	CMRenderer::CMRenderer(CMRendererSettings settings) noexcept
		: m_CMLogger(S_LIFETIME_LOG_FILE_NAME),
		  m_Settings(settings),
		  m_Window(m_Settings, m_CMLogger),
		  m_RenderContext(m_CMLogger)
	{
		m_CMLogger.LogInfoNL(L"CMRenderer [()] | Constructed.");
	}

	CMRenderer::~CMRenderer() noexcept
	{
		if (!m_Shutdown)
			Shutdown();

		m_CMLogger.LogInfoNL(L"CMRenderer [~()] | Destroyed.");
	}

#pragma region Public
	void CMRenderer::Init() noexcept
	{
		if (m_Initialized)
		{
			m_CMLogger.LogWarningNL(L"CMRenderer [Init] | Attempted to initialize after initialization has already occured previously.");
			return;
		}

		m_Window.Init();
		m_RenderContext.Init(m_Window.Handle(), m_Window.ClientArea());

		m_Initialized = true;
		m_Shutdown = false;
	}

	void CMRenderer::Shutdown() noexcept
	{
		if (m_Shutdown)
		{
			m_CMLogger.LogWarningNL(L"CMRenderer [Shutdown] | Attempted to shutdown after shutdown has already occured previously.");
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

		CMKeyboard& keyboardRef = m_Window.Keyboard();

		float angle = 0.0f;
		while (m_Window.IsRunning())
		{
			m_Window.HandleMessages();

			// Quit message has been received.
			if (!m_Window.IsRunning())
				break;

			if (keyboardRef.IsReleasedClear('i'))
			{
				if (!m_Window.IsWindowed())
					m_Window.Restore();
				else
					m_CMLogger.LogInfoNL(L"CMRenderer [CharKeyCallback ('I')] | Windowed key was released, but window is already windowed.");
			}
			else if (keyboardRef.IsReleasedClear('o'))
			{
				if (!m_Window.IsMaximized())
					m_Window.Maximize();
				else
					m_CMLogger.LogInfoNL(L"CMRenderer [CharKeyCallback ('O')] | Maximize key was released, but window is already maximized.");
			}
			else if (keyboardRef.IsReleasedClear('p'))
			{
				if (!m_Window.IsMinimized())
					m_Window.Minimize();
				else
					m_CMLogger.LogInfoNL(L"CMRenderer [CharKeyCallback ('P')] | Minimize key was pressed, but window is already minimized.");
			}

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