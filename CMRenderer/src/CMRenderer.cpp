#include "Core/CMPCH.hpp"
#include "CMRenderer.hpp"

#include <roapi.h>

namespace CMRenderer
{
	CMRenderer::CMRenderer(CMRendererSettings settings) noexcept
		: m_CMLogger(S_LIFETIME_LOG_FILE_NAME),
		  m_Settings(settings),
		  m_Window(m_Settings, m_CMLogger),
		  m_RenderContext(m_CMLogger, m_Settings.WindowSettings.Current)
	{
		/*
		 * For Microsoft DirectX applications, you must initialize the initial 
		 * thread by using Windows::Foundation::Initialize(RO_INIT_MULTITHREADED).
		 */
		HRESULT hResult = Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);

		if (hResult != S_OK)
			m_CMLogger.LogFatalNL(L"CMRenderer [()] | Failed to initialize COM and Windows Runtime API's.");

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
		if (m_Initialized)
		{
			m_CMLogger.LogWarningNL(L"CMRenderer [Init] | Attempted to initialize after initialization has already occured previously.");
			return;
		}

		m_Window.Init();
		m_RenderContext.Init(m_Window.Handle());

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
		CMKeyboard& keyboardRef = m_Window.Keyboard();

		/*constexpr float ROTATION_CONSTANT = 1.0f;
		constexpr float ROTATION_DEFAULT = 30.0f;
		float rotAngleX = 0.0f;
		float rotAngleY = ROTATION_DEFAULT;

		constexpr float OFFSET_CONSTANT = 0.5f;
		float offsetX = 0.0f;
		float offsetY = 0.0f;*/

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
					m_CMLogger.LogInfoNL(L"CMRenderer [Run] | Windowed key was released, but window is already windowed.");
			}
			else if (keyboardRef.IsReleasedClear('o'))
			{
				if (!m_Window.IsMaximized())
					m_Window.Maximize();
				else
					m_CMLogger.LogInfoNL(L"CMRenderer [Run] | Maximize key was released, but window is already maximized.");
			}
			else if (keyboardRef.IsReleasedClear('p'))
			{
				if (!m_Window.IsMinimized())
					m_Window.Minimize();
				else
					m_CMLogger.LogInfoNL(L"CMRenderer [Run] | Minimize key was pressed, but window is already minimized.");
			}
			/*else
			{
				if (keyboardRef.IsPressed('w'))
					offsetY -= OFFSET_CONSTANT;
				if (keyboardRef.IsPressed('s'))
					offsetY += OFFSET_CONSTANT;
				if (keyboardRef.IsPressed('a'))
					offsetX += OFFSET_CONSTANT;
				if (keyboardRef.IsPressed('d'))
					offsetX -= OFFSET_CONSTANT;
				if (keyboardRef.IsPressed('1'))
					rotAngleX += ROTATION_CONSTANT;
				if (keyboardRef.IsPressed('2'))
					rotAngleX -= ROTATION_CONSTANT;
				if (keyboardRef.IsPressed('3'))
					rotAngleY += ROTATION_CONSTANT;
				if (keyboardRef.IsPressed('4'))
					rotAngleY -= ROTATION_CONSTANT;
				else if (keyboardRef.IsPressed('r'))
				{
					offsetX = 0.0f;
					offsetY = 0.0f;

					rotAngleX = 0.0f;
					rotAngleY = ROTATION_DEFAULT;
				}*/
			}

			m_RenderContext.Clear({ 0.0f, 0.0f, 0.0f, 0.0f });
			//m_RenderContext.TestDraw(rotAngleX, rotAngleY, offsetX, offsetY);
			m_RenderContext.TestTextureDraw();
			m_RenderContext.Present();

			Sleep(10);
		}
	}
#pragma endregion
#pragma region Private

#pragma endregion
}