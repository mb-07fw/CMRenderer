#include "CMR_PCH.hpp"
#include "CMR_Window.hpp"

#include "CMC_Utility.hpp"
#include "CMC_WindowsUtility.hpp"

#include <chrono>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace CMRenderer
{
#pragma region Public
	CMWindow::CMWindow(CMCommon::CMLoggerWide& cmLoggerRef, CMRendererSettings& cmSettingsRef, std::function<void()> onResizeFunc) noexcept
		: m_CMLoggerRef(cmLoggerRef),
		  m_CMSettingsRef(cmSettingsRef),
		  m_WindowSettingsRef(cmSettingsRef.WindowSettings),
		  m_Keyboard(cmLoggerRef),
		  m_OnResizeFunc(onResizeFunc)
	{
		ValidateSettings();
		LogCurrentSettings();

		m_hInstance = GetCurrentHINSTANCE();

		m_CMLoggerRef.LogInfoNL(L"CMWindow [()] Initialized.");
	}

	CMWindow::~CMWindow() noexcept
	{
		if (!m_Shutdown)
			Shutdown();

		m_CMLoggerRef.LogInfoNL(L"CMWindow [~()] | Destroyed.");
	}

	void CMWindow::Init() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Initialized, L"CMWindow [Init] | Initialization has been attempted after CMWindow has already been initialized.");

		bool createdWindow = Create();

		m_CMLoggerRef.LogFatalNLIf(!createdWindow, L"CMWindow [Init] | Failed to create the window.");

		m_Initialized = true;
		m_Shutdown = false;

		m_CMLoggerRef.LogInfoNL(L"CMWindow [Init] | Initialzed.");
	}

	void CMWindow::Shutdown() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Shutdown, L"CMWindow [Shutdown] | Shutdown has been attempted after CMWindow has already been shutdown.");
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"CMWindow [Shutdown] | Shutdown has been attempted before CMWindow has been initialized.");

		bool destroyed = Destroy();

		m_CMLoggerRef.LogFatalNLIf(!destroyed, L"CMWindow [Shutdown] | Failed to destroy the window.");
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
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"CMWindow [Maximize] | Attempted to maximize the window before initializing it.");

		if (m_CMLoggerRef.LogWarningNLIf(m_CurrentState == CMWindowStateType::MAXIMIZED, L"CMWindow [Minimize] | Attempted to maximized the window while it is already in maximized state."))
			return;

		SetWindowLongPtrW(m_WindowHandle, GWL_STYLE, WS_POPUP);

		//  If nonzero, the return value is nonzero. Otherwise, the window was previously hidden.
		ShowWindow(m_WindowHandle, SW_SHOWMAXIMIZED);

		m_CMLoggerRef.LogInfoNL(L"CMWindow [Maximize] | Maximized window.");

		m_PreviousState = m_CurrentState;
		m_CurrentState = CMWindowStateType::MAXIMIZED;
	}

	void CMWindow::Minimize() noexcept
	{
		if (m_CMLoggerRef.LogWarningNLIf(!m_Initialized, L"CMWindow [Minimize] | Attempted to minimize the window before initializing it.") ||
			m_CMLoggerRef.LogWarningNLIf(m_CurrentState == CMWindowStateType::MINIMIZED, L"CMWindow [Minimize] | Attempted to minimize the window while it is already in minimized state."))
			return;

		//  If nonzero, the return value is nonzero. Otherwise, the window was previously hidden.
		ShowWindow(m_WindowHandle, SW_SHOWMINIMIZED);
		m_CMLoggerRef.LogInfoNL(L"CMWindow [Minimize] | Minimized window.");

		m_PreviousState = m_CurrentState;
		m_CurrentState = CMWindowStateType::MINIMIZED;
	}

	void CMWindow::Restore() noexcept
	{
		if (m_CMLoggerRef.LogWarningNLIf(!m_Initialized, L"CMWindow [Restore] | Attempted to restore the window before initializing it.") ||
			m_CMLoggerRef.LogWarningNLIf(m_CurrentState == CMWindowStateType::WINDOWED, L"CMWindow [Restore] | Attempted to restore the window when it is already in windowed state."))
			return;

		LONG_PTR currentStyle = GetWindowLongPtrW(m_WindowHandle, GWL_STYLE);

		if (!m_CMLoggerRef.LogWarningNLIf(currentStyle == 0, L"CMWindow [Restore] | Failed to retrieve window style."))
		{
			// Bitwise witchery.
			if (currentStyle & WS_POPUP)
			{
				m_CMLoggerRef.LogInfoNL(L"CMWindow [Restore] | WS_POPUP is set.");

				// If the function succeeds, the return value is the previous value of the specified offset.
				// If the function fails, the return value is zero.To get extended error information, call GetLastError.
				if (SetWindowLongPtrW(m_WindowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW) == 0)
					m_CMLoggerRef.LogInfoNL(L"CMWindow [Restore] | Failed to set windowed style.");
				else
					m_CMLoggerRef.LogInfoNL(L"CMWindow [Restore] | Set windowed style.");
			}

			BOOL succeeded = SetWindowPos(m_WindowHandle, nullptr, 0, 0, m_WindowSettingsRef.Current.InitialWidth, m_WindowSettingsRef.Current.InitialHeight, SWP_NOMOVE);

			if (!succeeded)
				m_CMLoggerRef.LogWarningNL(L"CMWindow [Restore] | Failed to set windowed pos.");
			else
				m_CMLoggerRef.LogInfoNL(L"CMWindow [Restore] | Set windowed pos.");
		}

		ShowWindow(m_WindowHandle, SW_RESTORE);

		m_CMLoggerRef.LogInfoNL(L"CMWindow [Restore] | Restored window.");

		m_PreviousState = m_CurrentState;
		m_CurrentState = CMWindowStateType::WINDOWED;
	}

	[[nodiscard]] CMWindowResolution CMWindow::CurrentResolution() const noexcept
	{
		return CMWindowResolution { 
			.Width = m_WindowSettingsRef.Current.ClientArea.right,
			.Height = m_WindowSettingsRef.Current.ClientArea.bottom 
		};
	}
#pragma endregion

#pragma region Private
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
		wndClass.lpfnWndProc = WndProcSetup;	/* Set the window procedure to the static setup function that will eventually thunk (redirect / forward)
												 *   Window's messages to an instance function that has access to Window and Renderer state.
												 *	  
												 *   The function assigned to lpfnWndProc has to be static due to how instance functions work in C++,
												 *   and how windows procedures are defined in the WinAPI.
												 *	  
												 *   For context, Instance functions implicitly take in an argument of this, which is incompatible with
												 *   the WinAPI window procedure standard, thus static functions are required as they omit the this argument.
												 *	  
												 *   In conclusion, the indirection is required to be able to access CTMRenderer::Window state in the window procedure without
												 *   the introduction of global state.
												 */

		wndClass.hInstance = m_hInstance;
		wndClass.lpszClassName = S_CLASS_NAME.data();

		RegisterWindowClass(wndClass);

		RECT& clientAreaRef = m_WindowSettingsRef.Current.ClientArea;

		clientAreaRef.right = m_WindowSettingsRef.Current.InitialWidth;
		clientAreaRef.bottom = m_WindowSettingsRef.Current.InitialHeight;

		DWORD dwStyle = m_WindowSettingsRef.Current.UseFullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;

		// Create the window.
		m_WindowHandle = CreateWindowExW(
			0,												 // 0 for no optional window styles.
			wndClass.lpszClassName,							 // Window class.
			m_WindowSettingsRef.Current.WindowTitle.data(),	 // Window title.
			dwStyle,										 // Window styles. 
			CW_USEDEFAULT,									 // X position.
			CW_USEDEFAULT,									 // Y position.
			clientAreaRef.right - clientAreaRef.left,		 // Window width. 
			clientAreaRef.bottom - clientAreaRef.top,		 // Window height.
			nullptr,										 // Parent window.
			nullptr,										 // Menu.
			wndClass.hInstance,								 // Handle to the instance of the application. (HINSTANCE)
			this											 // Other optional program data. (LPVOID)

			/* NOTE : We pass in @this to be able to encapsulate the WndProc
			 *        as a instance member function so we have access to CMWindow state.
			 */
		);

		m_CMLoggerRef.LogFatalNLIf(m_WindowHandle == nullptr, L"CMWindow [Create] | Failed to get a valid window handle. Window handle is nullptr.");

		// Here so Intelisense doesn't yell at me for de-referencing a nullptr, even though fatal logs terminate the program.
		if (m_WindowHandle == nullptr)
			return false;

		int showCmd = m_WindowSettingsRef.Current.UseFullscreen ? SW_SHOWMAXIMIZED : SW_SHOW;

		//  If nonzero, the return value is nonzero. Otherwise, the window was previously hidden.
		BOOL previouslyVisible = ShowWindow(m_WindowHandle, showCmd);

		BOOL result = GetClientRect(m_WindowHandle, &clientAreaRef);

		m_CMLoggerRef.LogFatalNLIf(!result, L"CMWindow [Create] | Failed to update window size.");

		m_CurrentState = m_WindowSettingsRef.Current.UseFullscreen ? CMWindowStateType::MAXIMIZED :
			CMWindowStateType::WINDOWED;

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
		m_CMLoggerRef.LogFatalNLIf(destroyedWindow == 0, L"CMWindow [Destroy] | Failed to destroy the window. Please help me.");

		int unregisteredClass = UnregisterClassW(S_CLASS_NAME.data(), m_hInstance);
		m_CMLoggerRef.LogFatalNLIf(unregisteredClass == 0, L"CMWindow [Destroy] | Failed to unregisted the window's class. How did you manage to do this???");

		return true;
	}

	[[nodiscard]] HINSTANCE CMWindow::GetCurrentHINSTANCE() noexcept
	{
		HMODULE hModule = nullptr;

		// If this parameter is NULL, the function returns a handle to the file used to create the calling process (.exe file).
		BOOL succeeded = GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, NULL, &hModule);

		m_CMLoggerRef.LogFatalNLIf(succeeded == 0, L"CMWindow [GetCurrentHINSTANCE] | Call to GetModuleHandleExW failed.");
		m_CMLoggerRef.LogFatalNLIf(hModule == nullptr, L"CMWindow [GetCurrentHINSTANCE] | Unable to retrieve the current HINSTANCE. Retrieved HINSTANCE was nullptr.");

		return static_cast<HINSTANCE>(hModule);
	}

	void CMWindow::RegisterWindowClass(const WNDCLASSEX& wndClassRef) noexcept
	{
		/* https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		 * If the function succeeds, the return value is a class atom that uniquely identifies the class being registered.
		 * This atom can only be used by the CreateWindow, CreateWindowEx, GetClassInfo, GetClassInfoEx, FindWindow,
		 * FindWindowEx, and UnregisterClass functions and the IActiveIMMap::FilterClientWindows method.

		 * If the function fails, the return value is zero. To get extended error information, call GetLastError.
		 */
		bool registeredClass = RegisterClassExW(&wndClassRef);
		m_CMLoggerRef.LogFatalNLIf(!registeredClass, L"CMWindow [RegisterWindowClass] | Failed to register CMWindow's WNDCLASSEX.");
	}
#pragma endregion

#pragma region Settings
	void CMWindow::ValidateSettings() noexcept
	{
		if (m_CMLoggerRef.LogWarningNLAppendIf(
			m_WindowSettingsRef.Current.WindowTitle.data() == nullptr,
			L"CMWindow [ValidateSettings] | Window title is nullptr. "
			L"Resorting to default : ", CMWindowData::S_DEFAULT_WINDOW_TITLE.data()
		))
			m_WindowSettingsRef.SetTitleToDefault();

		else if (m_CMLoggerRef.LogWarningNLAppendIf(
			m_WindowSettingsRef.Current.WindowTitle.size() == 0,
			L"CMWindow [ValidateSettings] | Window title has a character size of 0. "
			L"Resorting to default : ", CMWindowData::S_DEFAULT_WINDOW_TITLE.data()
		))
			m_WindowSettingsRef.SetTitleToDefault();

		if (m_CMLoggerRef.LogWarningNLVariadicIf(
			m_WindowSettingsRef.Current.InitialWidth <= 0,
			L"CMWindow [ValidateSettings] | Window width is negative. "
			L"(Width : ", m_WindowSettingsRef.Current.InitialWidth,
			L") Resorting to default : ", CMWindowData::S_DEFAULT_WIDTH
		))
			m_WindowSettingsRef.SetWidthToDefault();

		else if (m_CMLoggerRef.LogWarningNLVariadicIf(
			m_WindowSettingsRef.Current.InitialWidth >= m_WindowSettingsRef.MaxWidth,
			L"CMWindow [ValidateSettings] | Window width is greater than max width. "
			L"(Width : ", m_WindowSettingsRef.Current.InitialWidth,
			L" | Max Width : ", m_WindowSettingsRef.MaxWidth,
			L") Resorting to default: ", CMWindowData::S_DEFAULT_WIDTH
		))
			m_WindowSettingsRef.SetWidthToDefault();

		if (m_CMLoggerRef.LogWarningNLVariadicIf(
			m_WindowSettingsRef.Current.InitialHeight <= 0,
			L"CMWindow [ValidateSettings] | Window height is negative. "
			L"(Height : ", m_WindowSettingsRef.Current.InitialHeight,
			L") Resorting to default : ", CMWindowData::S_DEFAULT_HEIGHT
		))
			m_WindowSettingsRef.SetHeightToDefault();

		else if (m_CMLoggerRef.LogWarningNLVariadicIf(
			m_WindowSettingsRef.Current.InitialHeight >= m_WindowSettingsRef.MaxHeight,
			L"CMWindow [ValidateSettings] | Window height is greater than max width. "
			L"(Height : ", m_WindowSettingsRef.Current.InitialHeight,
			L" | Max Height : ", m_WindowSettingsRef.MaxHeight, 
			L") Resorting to default: ", CMWindowData::S_DEFAULT_HEIGHT
		))
			m_WindowSettingsRef.SetHeightToDefault();
		
		m_WindowSettingsRef.Current.ClientArea = { 0, 0, m_WindowSettingsRef.Current.InitialWidth, m_WindowSettingsRef.Current.InitialHeight };
	}

	void CMWindow::LogCurrentSettings() const noexcept
	{
		m_CMLoggerRef.LogInfoNLAppend(
			L"CMWindow [LogCurrentSettings] | Target Title : ",
			m_WindowSettingsRef.Current.WindowTitle
		);

		m_CMLoggerRef.LogInfoNLVariadic(
			L"CMWindow [LogCurrentSettings] | Target Window Resolution : ",
			m_WindowSettingsRef.Current.InitialWidth, L" x ", m_WindowSettingsRef.Current.InitialHeight
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
		if (ImGui_ImplWin32_WndProcHandler(m_WindowHandle, msgCode, wParam, lParam))
			return true;

		switch (msgCode)
		{
		case WM_KEYDOWN:

			m_Keyboard.SetPressedVK(static_cast<BYTE>(wParam));
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		case WM_KEYUP:
			if (wParam != VK_ESCAPE)
			{
				m_Keyboard.SetReleasedVK(static_cast<BYTE>(wParam));
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
			/*if (wParam == VK_RETURN && (GetKeyState(VK_MENU) & 0x8000) != 0)
				return S_OK;*/

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
			GetClientRect(m_WindowHandle, &m_WindowSettingsRef.Current.ClientArea);
			m_OnResizeFunc();

			[[fallthrough]];
		default:
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		}
	}
#pragma endregion
#pragma endregion
}