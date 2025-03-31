#include "Core/CMPCH.hpp"
#include "Internal/CMWindow.hpp"

#include "Core/Utility.hpp"

#include <chrono>

namespace CMRenderer
{
#pragma region Public
	CMWindow::CMWindow(CMRendererSettings& cmSettingsRef, CMLoggerWide& cmLoggerRef) noexcept
		: m_CMSettingsRef(cmSettingsRef), m_WindowSettingsRef(cmSettingsRef.WindowSettings),
		m_CMLoggerRef(cmLoggerRef)
	{
		ValidateSettings();
		LogCurrentSettings();

		m_hInstance = GetCurrentHINSTANCE();
	}

	CMWindow::~CMWindow() noexcept
	{
		if (!m_Shutdown)
			Shutdown();
	}

	void CMWindow::Init() noexcept
	{
		if (m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Init] | Initialization has been attempted after CMWindow has already been initialized.");
			return;
		};

		bool createdWindow = Create();

		if (!createdWindow)
			m_CMLoggerRef.LogFatalNL(L"CMWindow [Init] | Failed to create the window.");

		m_Initialized = true;
		m_Shutdown = false;
	}

	void CMWindow::Shutdown() noexcept
	{
		if (m_Shutdown)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Shutdown] | Shutdown has been attempted after CMWindow has already been shutdown.");
			return;
		}
		else if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Shutdown] | Shutdown has been attempted before CMWindow has been initialized.");
			return;
		}

		bool destroyed = Destroy();

		if (!destroyed)
			m_CMLoggerRef.LogFatalNL(L"CMWindow [Shutdown] | Failed to destroy the window.");

		m_Initialized = false;
		m_Shutdown = true;

		m_CMLoggerRef.LogInfoNL(L"CMWindow [Shutdown] | Shutdown.");
	}

	void CMWindow::HandleMessages() noexcept
	{
		BOOL result = 0;
		MSG msg = {};

		// While there are messages to process. (Return value is non-zero)
		while ((result = PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) != 0)
		{
			if (msg.message == WM_QUIT)
				SetShutdownState();

			// Translate any raw virtual-key messages in character messages. (e.g., 'w', 'a', 's', 'd', etc)
			TranslateMessage(&msg);

			// Forward the message to the current window procedure.
			DispatchMessageW(&msg);
		}
	}

	void CMWindow::WaitForMessages(long long handleMillis) noexcept
	{
		bool firstMessage = false;
		std::chrono::steady_clock::time_point firstRetrievalTime;

		BOOL result = 0;
		MSG msg = {};

		while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
		{
			if (!firstMessage)
			{
				firstMessage = true;
				firstRetrievalTime = std::chrono::high_resolution_clock::now();
			}

			// Translate any raw virtual-key messages in character messages. (e.g., 'w', 'a', 's', 'd', etc)
			TranslateMessage(&msg);

			// Forward the message to the current window procedure.
			DispatchMessageW(&msg);

			std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - firstRetrievalTime);

			if (elapsed.count() >= handleMillis)
				return;
		}

		if (msg.message == WM_QUIT)
			SetShutdownState();
	}

	void CMWindow::Maximize() noexcept
	{
		if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Maximize] | Attempted to maximize the window before initializing it.");
			return;
		}

		if (m_Maximized)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Minimize] | Attempted to maximized the window while it is already in maximized state.");
			return;
		}

		SetWindowLongPtrW(m_WindowHandle, GWL_STYLE, WS_POPUP);
		ShowWindow(m_WindowHandle, SW_SHOWMAXIMIZED);

		m_CMLoggerRef.LogInfoNL(L"CMWindow [Maximize] | Maximized window.");

		m_Maximized = true;
		m_Minimized = false;
		m_Windowed = false;
	}

	void CMWindow::Minimize() noexcept
	{
		if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Minimize] | Attempted to minimize the window before initializing it.");
			return;
		}

		if (m_Minimized)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Minimize] | Attempted to minimize the window while it is already in minimized state.");
			return;
		}

		ShowWindow(m_WindowHandle, SW_SHOWMINIMIZED);
		m_CMLoggerRef.LogInfoNL(L"CMWindow [Minimize] | Minimized window.");

		m_Maximized = false;
		m_Minimized = true;
		m_Windowed = false;
	}

	void CMWindow::Restore() noexcept
	{
		if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Restore] | Attempted to restore the window before initializing it.");
			return;
		}

		if (m_Windowed)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Restore] | Attempted to restore the window when it is already in windowed state.");
			return;
		}

		LONG_PTR currentStyle = GetWindowLongPtrW(m_WindowHandle, GWL_STYLE);

		if (currentStyle == 0)
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Restore] | Failed to retrieve window style.");
		else
		{
			if (currentStyle & WS_POPUP)
			{
				m_CMLoggerRef.LogInfoNL(L"CMWindow [Restore] | WS_POPUP is set.");

				SetWindowLongPtrW(m_WindowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);

				m_CMLoggerRef.LogInfoNL(L"CMWindow [Restore] | Set windowed style.");
			}

			BOOL succeeded = SetWindowPos(m_WindowHandle, nullptr, 0, 0, m_WindowSettingsRef.Current.Width, m_WindowSettingsRef.Current.Height, SWP_NOMOVE);

			if (!succeeded)
				m_CMLoggerRef.LogWarningNL(L"CMWindow [Restore] | Failed to set windowed pos.");
			else
				m_CMLoggerRef.LogInfoNL(L"CMWindow [Restore] | Set windowed pos.");
		}

		ShowWindow(m_WindowHandle, SW_RESTORE);
		m_CMLoggerRef.LogInfoNL(L"CMWindow [Restore] | Restored window.");

		m_Maximized = false;
		m_Minimized = false;
		m_Windowed = true;
	}

	void CMWindow::SetCharKeyCallback(char c, std::function<void(bool)> func) noexcept
	{
		m_CharKeyCallbacks[c] = func;
	}
#pragma endregion

#pragma region Private
	void CMWindow::CallCharKeyCallback(char c, bool isReleased) noexcept
	{
		std::function<void(bool)> func = m_CharKeyCallbacks[c];

		if (!func)
			return;

		func(isReleased);
	}
#pragma region State
	void CMWindow::SetShutdownState() noexcept
	{
		m_Running = false;
		m_CMLoggerRef.LogInfoNL(L"CMWindow [SetShutdownState] | Window shutdown has been requested.");

		Minimize();
	}
#pragma endregion

#pragma region Window Management
	bool CMWindow::Create() noexcept
	{
		WNDCLASSEXW wndClass = {};

		wndClass.cbSize = sizeof(WNDCLASSEXW);
		wndClass.lpfnWndProc = WndProcSetup; /* Set the window procedure to the static setup function that will eventually thunk (redirect / forward)
											  *  Window's messages to an instance function that has access to Window and Renderer state.
											  *
											  *  The function assigned to lpfnWndProc has to be static due to how instance functions work in C++,
											  *  and how windows procedures are defined in the WinAPI.
											  *
											  *  For context, Instance functions implicitly take in an argument of this, which is incompatible with
											  *  the WinAPI window procedure standard, thus static functions are required as they omit the this argument.
											  *
											  *  In conclusion, the indirection is required to be able to access CTMRenderer::Window state in the window procedure without
											  *  the introduction of global state.
											  */

		wndClass.hInstance = m_hInstance;
		wndClass.lpszClassName = S_CLASS_NAME.data();

		RegisterWindowClass(wndClass);

		m_ClientArea.left = 0;
		m_ClientArea.right = m_WindowSettingsRef.Current.Width;
		m_ClientArea.top = 0;
		m_ClientArea.bottom = m_WindowSettingsRef.Current.Height;

		DWORD dwStyle = m_WindowSettingsRef.Current.UseFullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;

		// Create the window.
		m_WindowHandle = CreateWindowExW(
			0,														// 0 for no optional window styles.
			wndClass.lpszClassName,									// Window class.
			m_WindowSettingsRef.Current.WindowTitle.data(),			// Window title.
			dwStyle,												// Window styles. 
			CW_USEDEFAULT, CW_USEDEFAULT,							// X, Y position.
			m_ClientArea.right - m_ClientArea.left,					// Window width. 
			m_ClientArea.bottom - m_ClientArea.top,					// Window height.
			nullptr,												// Parent window.
			nullptr,												// Menu.
			wndClass.hInstance,										// Handle to the instance of the application. (HINSTANCE)
			this													// Other optional program data. (LPVOID)

			/* NOTE : We pass in @this to be able to encapsulate the WndProc
			 *        as a instance member function so we have access to CMWindow state.
			 */
		);

		if (m_WindowHandle == nullptr)
		{
			m_CMLoggerRef.LogFatalNL(L"CMWindow [Create] | Failed to get a valid window handle. Window handle is nullptr.");
			return false;
		}

		int showCmd = m_WindowSettingsRef.Current.UseFullscreen ? SW_SHOWMAXIMIZED : SW_SHOW;

		// If nonzero, the return value is nonzero. Otherwise, the window was previously hidden.
		BOOL previouslyVisible = ShowWindow(m_WindowHandle, showCmd);

		BOOL result = GetClientRect(m_WindowHandle, &m_ClientArea);
		
		if (!result)
		{
			m_CMLoggerRef.LogFatalNL(L"CMWindow [Create] | Failed to update window size.");
			return false;
		}

		m_Maximized = m_WindowSettingsRef.Current.UseFullscreen;
		m_Windowed = !m_Maximized;
		m_Running = true;
		return true;
	}

	bool CMWindow::Destroy() noexcept
	{
		if (m_Running)
		{
			PostQuitMessage(0);
			m_Running = false;
		}

		int destroyedWindow = DestroyWindow(m_WindowHandle);

		if (destroyedWindow == 0)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Destroy] | Failed to destroy the window handle.");
			return false;
		}

		int unregisteredClass = UnregisterClassW(S_CLASS_NAME.data(), m_hInstance);

		if (unregisteredClass == 0)
		{
			m_CMLoggerRef.LogWarningNL(L"CMWindow [Destroy] | Failed to unregisted the window's class. How did you manage to do this???");
			return false;
		}

		return true;
	}

	[[nodiscard]] HINSTANCE CMWindow::GetCurrentHINSTANCE() noexcept
	{
		HMODULE hModule = nullptr;
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, NULL, &hModule);

		if (hModule == nullptr)
			m_CMLoggerRef.LogFatalNL(L"CMWindow [GetCurrentHINSTANCE] | Unable to retrieve the current HINSTANCE. Retrieved HINSTANCE was nullptr.");

		return static_cast<HINSTANCE>(hModule);
	}

	void CMWindow::RegisterWindowClass(const WNDCLASSEX& wndClassRef) noexcept
	{
		bool registeredClass = RegisterClassExW(&wndClassRef);

		if (!registeredClass)
			m_CMLoggerRef.LogFatalNL(L"CMWindow [RegisterWindowClass] | Failed to register CMWindow's WNDCLASSEX.");
	}
#pragma endregion

#pragma region Settings
	void CMWindow::ValidateSettings() noexcept
	{
		if (m_WindowSettingsRef.Current.WindowTitle.data() == nullptr)
		{
			m_CMLoggerRef.LogWarningNLAppend(
				L"CMWindow [ValidateSettings] | Window title is nullptr."
				"Resorting to default : ", CMWindowData::S_DEFAULT_WINDOW_TITLE.data()
			);

			m_WindowSettingsRef.SetTitleToDefault();
		}

		else if (m_WindowSettingsRef.Current.WindowTitle.size() == 0)
		{
			m_CMLoggerRef.LogWarningNLAppend(
				L"CMWindow [ValidateSettings] | Window title has a size of 0."
				"Resorting to default : ", CMWindowData::S_DEFAULT_WINDOW_TITLE.data()
			);

			m_WindowSettingsRef.SetTitleToDefault();
		}

		if (m_WindowSettingsRef.Current.Width <= 0)
		{
			m_CMLoggerRef.LogWarningNLVariadic(
				L"CMWindow [ValidateSettings] | Window width is negative."
				"(Width : ", m_WindowSettingsRef.Current.Width,
				L") Resorting to default : ", CMWindowData::S_DEFAULT_WIDTH
			);

			m_WindowSettingsRef.SetWidthToDefault();
		}

		else if (m_WindowSettingsRef.Current.Width >= m_WindowSettingsRef.MaxWidth)
		{
			m_CMLoggerRef.LogWarningNLVariadic(
				L"CMWindow [ValidateSettings] | Window width is greater than max width."
				"(Width : ", m_WindowSettingsRef.Current.Width,
				L" | Max Width : ", m_WindowSettingsRef.MaxWidth,
				L") Resorting to default: ", CMWindowData::S_DEFAULT_WIDTH
			);

			m_WindowSettingsRef.SetWidthToDefault();
		}

		if (m_WindowSettingsRef.Current.Height <= 0)
		{
			m_CMLoggerRef.LogWarningNLVariadic(
				L"CMWindow [ValidateSettings] | Window height is negative."
				"(Height : ", m_WindowSettingsRef.Current.Height,
				L") Resorting to default : ", CMWindowData::S_DEFAULT_HEIGHT
			);

			m_WindowSettingsRef.SetHeightToDefault();
		}

		else if (m_WindowSettingsRef.Current.Height >= m_WindowSettingsRef.MaxHeight)
		{
			m_CMLoggerRef.LogWarningNLVariadic(
				L"CMWindow [ValidateSettings] | Window height is greater than max width."
				"(Height : ", m_WindowSettingsRef.Current.Height,
				L" | Max Height : ", m_WindowSettingsRef.MaxHeight, 
				L") Resorting to default: ", CMWindowData::S_DEFAULT_HEIGHT
			);

			m_WindowSettingsRef.SetHeightToDefault();
		}
	}

	void CMWindow::LogCurrentSettings() const noexcept
	{
		m_CMLoggerRef.LogInfoNLAppend(
			L"CMWindow [LogCurrentSettings] | Target Title : ",
			m_WindowSettingsRef.Current.WindowTitle
		);

		m_CMLoggerRef.LogInfoNLVariadic(
			L"CMWindow [LogCurrentSettings] | Target Window Resolution : ",
			m_WindowSettingsRef.Current.Width, L" x ", m_WindowSettingsRef.Current.Height
		);

		m_CMLoggerRef.LogInfoNLVariadic(
			L"CMWindow [LogCurrentSettings] | Max Window Resolution : ",
			m_WindowSettingsRef.MaxWidth, L" x ", m_WindowSettingsRef.MaxHeight
		);

		m_CMLoggerRef.LogInfoNLAppend(
			L"CMWindow [LogCurrentSettings] | Use fullscreen : ", 
			Utility::BoolToWStrView(m_WindowSettingsRef.Current.UseFullscreen)
		);
	}
#pragma endregion

#pragma region WndProc
	[[nodiscard]] LRESULT CALLBACK CMWindow::WndProcSetup(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept
	{
		// If we get a message before the WM_NCCREATE message, handle with default window procedure provided by the WinAPI.
		// (WM_NCCREATE contains the instance of CMWindow that was passed to CreateWindowEx)
		if (msgCode != WM_NCCREATE)
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);

		// Retrieve create parameter passed into CreateWindowEx that stores an instance of CMWindow.
		const CREATESTRUCTW* const pCreateStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
		CMWindow* const pCMWindow = static_cast<CMWindow*>(pCreateStruct->lpCreateParams);

		// Set WinAPI-managed user data to store a pointer to CMWindow.
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCMWindow));

		// Set message procedure to WndProcThunk now that the instance of Window is stored.
		SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CMWindow::WndProcThunk));

		// Forward message to the instance WndProc.
		return pCMWindow->WndProc(hWnd, msgCode, wParam, lParam);
	}

	[[nodiscard]] LRESULT CALLBACK CMWindow::WndProcThunk(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept
	{
		// Retrieve pointer to CMWindow from WinAPI user data.
		CMWindow* const pCMWindow = reinterpret_cast<CMWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

		// Forward message to our CMWindow WndProc.
		return pCMWindow->WndProc(hWnd, msgCode, wParam, lParam);
	}

	[[nodiscard]] LRESULT CALLBACK CMWindow::WndProc(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (msgCode)
		{
		case WM_KEYDOWN:
			if (wParam != VK_ESCAPE)
			{
				CallCharKeyCallback(static_cast<char>(wParam), false);
				return DefWindowProcW(hWnd, msgCode, wParam, lParam);
			}

			[[fallthrough]];
		case WM_CLOSE:
			PostQuitMessage(0);
			return S_OK;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				m_CMLoggerRef.LogInfo(L"CMWindow [WndProc] Window deactivated.\n");
				m_Focused = false;
			}
			else if (LOWORD(wParam) == WA_ACTIVE)
			{
				m_CMLoggerRef.LogInfo(L"CMWindow [WndProc] Window activated.\n");
				m_Focused = true;
			}

			return S_OK;
		case WM_SYSKEYDOWN:
			// If alt + enter is pressed. ((GetKeyState(VK_MENU) & 0x8000) != 0 alt is held down)
			if (wParam == VK_RETURN && (GetKeyState(VK_MENU) & 0x8000) != 0)
				return S_OK;

			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		case WM_SYSCOMMAND:
			m_CMLoggerRef.LogInfo(L"CMWindow [WndProc] | Received SYSCOMMAND.\n");

			if ((wParam & 0xFFF0) == SC_MAXIMIZE)
				Maximize();
			else if ((wParam & 0xFFF0) == SC_MINIMIZE)
				Minimize();
			else if ((wParam & 0xFFF0) == SC_RESTORE)
				Restore();

			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		case WM_SIZE:
			GetClientRect(m_WindowHandle, &m_ClientArea);
			[[fallthrough]];
		default:
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		}
	}
#pragma endregion
#pragma endregion
}