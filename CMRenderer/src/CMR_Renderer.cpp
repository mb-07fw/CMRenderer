#include "CMR_PCH.hpp"
#include "CMR_Renderer.hpp"

namespace CMRenderer
{
	CMRenderer::CMRenderer(CMRendererSettings settings) noexcept
		: m_CMLogger(S_LIFETIME_LOG_FILE_NAME),
		  m_Settings(settings),
		  m_Window(m_CMLogger, m_Settings, [this]() { OnWindowResize(); }),
		  m_Context(m_CMLogger, m_Settings.WindowSettings.Current)
	{
		/*
		 *	For Microsoft DirectX applications, you must initialize the initial 
		 *	thread by using Windows::Foundation::Initialize(RO_INIT_MULTITHREADED).
		 */
		m_CMLogger.LogFatalNLIf(
			FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED)),
			L"CMRenderer [()] | Failed to call Windows::Foundation::Initialize."
		);

		m_CMLogger.LogInfoNL(L"CMRenderer [()] | Constructed.");
	}

	CMRenderer::~CMRenderer() noexcept
	{
		if (!m_Shutdown)
			Shutdown();

		Windows::Foundation::Uninitialize();

		m_CMLogger.LogInfoNL(L"CMRenderer [~()] | Destroyed.");
	}

#pragma region Public
	void CMRenderer::Init() noexcept
	{
		m_CMLogger.LogFatalNLIf(m_Initialized, L"CMRenderer [Init] | Attempted to initialize after initialization has already occured previously.");

		m_Window.Init();
		m_Context.Init(m_Window.Handle());

		m_Initialized = true;
		m_Shutdown = false;

		m_CMLogger.LogInfoNL(L"CMRenderer [Init] | Initialized.");
	}

	void CMRenderer::Shutdown() noexcept
	{
		m_CMLogger.LogFatalNLIf(m_Shutdown, L"CMRenderer [Shutdown] | Attempted to shutdown after shutdown has already occured previously.");

		m_Window.Shutdown();
		m_Context.Shutdown();

		m_Initialized = false;
		m_Shutdown = true;

		m_CMLogger.LogInfoNL(L"CMRenderer [Shutdown] | Shutdown.");
	}

	void CMRenderer::UpdateWindow() noexcept
	{
		m_Window.HandleMessages();
	}

	void CMRenderer::Clear(CMCommon::NormColor normalizedColor) noexcept
	{
		m_Context.Clear(normalizedColor);
	}

	void CMRenderer::Present() noexcept
	{
		m_Context.Present();
	}
#pragma endregion
#pragma region Private

	void CMRenderer::OnWindowResize() noexcept
	{
		m_Context.OnWindowResize();
	}
#pragma endregion
}