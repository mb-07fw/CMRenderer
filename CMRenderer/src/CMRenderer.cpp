#include "Core/CMPCH.hpp"
#include "CMRenderer.hpp"

namespace CMRenderer
{
	CMRenderer::~CMRenderer() noexcept
	{
		if (!m_Shutdown)
			Shutdown();

		m_CMLogger.LogInfo(L"CMRenderer [~()] | Destroyed.\n");
	}

	void CMRenderer::Init() noexcept
	{
		if (m_Initialized)
		{
			m_CMLogger.LogWarning(L"CMRenderer [Init] | Attempted to initialize after initialization has already occured previously.\n");
			return;
		}

		m_Window.Init();
		m_RenderContext.Init(m_Window.Handle(), m_Window.ClientArea(), m_Settings.WindowSettings.Current.UseFullscreen);

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
			
			m_RenderContext.Clear({ 0.5f, 0.0f, 0.5f, 0.0f });
			m_RenderContext.Draw(array, 3);
			m_RenderContext.Present();

			Sleep(5);
		}
	}
}