#include "Core/CMPCH.hpp"
#include "CMRenderer.hpp"

namespace CMRenderer
{
	CMRenderer::CMRenderer(CMRendererSettings settings) noexcept
		: m_CMLogger(S_LIFETIME_LOG_FILE_NAME),
		  m_Settings(settings),
		  m_Window(m_Settings, m_CMLogger),
		  m_RenderContext(m_CMLogger, m_Settings.WindowSettings.Current)
	{
		/*
		 *	For Microsoft DirectX applications, you must initialize the initial 
		 *	thread by using Windows::Foundation::Initialize(RO_INIT_MULTITHREADED).
		 */
		HRESULT hResult = Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);

		m_CMLogger.LogFatalNLIf(hResult != S_OK, L"CMRenderer [()] | Failed to call Windows::Foundation::Initialize.");

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
		m_RenderContext.Init(m_Window.Handle());

		m_Initialized = true;
		m_Shutdown = false;

		m_CMLogger.LogInfoNL(L"CMRenderer [Init] | Initialized.");
	}

	void CMRenderer::Shutdown() noexcept
	{
		m_CMLogger.LogFatalNLIf(m_Shutdown, L"CMRenderer [Shutdown] | Attempted to shutdown after shutdown has already occured previously.");

		m_Window.Shutdown();
		m_RenderContext.Shutdown();

		m_Initialized = false;
		m_Shutdown = true;

		m_CMLogger.LogInfoNL(L"CMRenderer [Shutdown] | Shutdown.");
	}

	void CMRenderer::Run() noexcept
	{
		CMKeyboard& keyboardRef = m_Window.Keyboard();

		/*constexpr float ROTATION_CONSTANT = 1.0f;
		constexpr float ROTATION_DEFAULT = 30.0f;
		float rotAngleX = 0.0f;
		float rotAngleY = 0.0f;
		float angle = 0.0f;

		constexpr float OFFSET_CONSTANT = 0.5f;

		float offsetX = 0.0f;
		float offsetY = 0.0f;
		float offsetZ = 0.0f;*/


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
					offsetZ += OFFSET_CONSTANT;
				if (keyboardRef.IsPressed('s'))
					offsetZ -= OFFSET_CONSTANT;
				if (keyboardRef.IsPressed('a'))
					offsetX -= OFFSET_CONSTANT;
				if (keyboardRef.IsPressed('d'))
					offsetX += OFFSET_CONSTANT;
				if (keyboardRef.IsPressed(' '))
					offsetY += OFFSET_CONSTANT;
				if (keyboardRef.IsPressedVK(VK_SHIFT))
					offsetY -= OFFSET_CONSTANT;
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
					offsetZ = 0.0f;

					rotAngleX = 0.0f;
					rotAngleY = 0.0f;
				}
			}*/

			m_RenderContext.Clear({ 0.0f, 0.0f, 0.0f, 0.0f });
			//m_RenderContext.TestDraw(rotAngleX, rotAngleY, offsetX, offsetY, offsetZ);
			//m_RenderContext.TestTextureDraw(rotAngleX, rotAngleY, offsetX, offsetY, offsetZ);
			m_RenderContext.Present();

			//angle += 0.05f;

			Sleep(10);
		}
	}
#pragma endregion
#pragma region Private

#pragma endregion
}