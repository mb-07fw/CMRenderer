#include "Core/PCH.hpp"
#include "Win/Win_Window.hpp"
#include "Win/Win_Utility.hpp"
#include "Common/Utility.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace CMEngine::Win
{
#pragma region Public
	Window::Window(
		Common::LoggerWide& logger,
		WindowSettings& windowSettings,
		std::function<void()> onResizeFunc
	) noexcept
		: m_Logger(logger),
		  m_WindowSettings(windowSettings),
		  m_Keyboard(logger),
		  m_OnResizeFunc(onResizeFunc)
	{
		ValidateSettings();
		LogCurrentSettings();

		mP_Instance = GetCurrentInstance();

		m_Logger.LogInfoNL(L"Window [()] Initialized.");
	}

	Window::~Window() noexcept
	{
		if (!m_Shutdown)
			Shutdown();

		m_Logger.LogInfoNL(L"Window [~()] | Destroyed.");
	}

	void Window::Init() noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Initialized,
			L"Window [Init] | Initialization has been attempted after Window has already been initialized."
		);

		m_Logger.LogFatalNLIf(
			!Create(),
			L"Window [Init] | Failed to create the window."
		);

		m_Initialized = true;
		m_Shutdown = false;

		m_Logger.LogInfoNL(L"Window [Init] | Initialzed.");
	}

	void Window::Shutdown() noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Shutdown,
			L"Window [Shutdown] | Shutdown has been attempted after Window has already been shutdown."
		);

		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Window [Shutdown] | Shutdown has been attempted before Window has been initialized."
		);

		m_Logger.LogFatalNLIf(
			!Destroy(),
			L"Window [Shutdown] | Failed to destroy the window."
		);

		m_Initialized = false;
		m_Shutdown = true;

		m_Logger.LogInfoNL(L"Window [Shutdown] | Shutdown.");
	}

	void Window::HandleMessages() noexcept
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

	void Window::WaitForMessages(long long handleMillis) noexcept
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

	void Window::Maximize() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Window [Maximize] | Attempted to maximize the window before initializing it."
		);

		if(m_Logger.LogWarningNLIf(
			m_CurrentState == WindowStateType::MAXIMIZED,
			L"Window [Minimize] | Attempted to maximized the window while it is already in maximized state."
		))
			return;

		SetWindowLongPtrW(m_WindowHandle, GWL_STYLE, WS_POPUP);

		//  If nonzero, the return value is nonzero. Otherwise, the window was previously hidden.
		ShowWindow(m_WindowHandle, SW_SHOWMAXIMIZED);

		m_Logger.LogInfoNL(L"Window [Maximize] | Maximized window.");

		m_PreviousState = m_CurrentState;
		m_CurrentState = WindowStateType::MAXIMIZED;
	}

	void Window::Minimize() noexcept
	{
		if (m_Logger.LogWarningNLIf(
			!m_Initialized,
			L"Window [Minimize] | Attempted to minimize the window before initializing it."
		))
			return;

		if (m_Logger.LogWarningNLIf(
			m_CurrentState == WindowStateType::MINIMIZED,
			L"Window [Minimize] | Attempted to minimize the window while it is already in minimized state."
		))
			return;

		//  If nonzero, the return value is nonzero. Otherwise, the window was previously hidden.
		ShowWindow(m_WindowHandle, SW_SHOWMINIMIZED);
		m_Logger.LogInfoNL(L"Window [Minimize] | Minimized window.");

		m_PreviousState = m_CurrentState;
		m_CurrentState = WindowStateType::MINIMIZED;
	}

	void Window::Restore() noexcept
	{
		if (m_Logger.LogWarningNLIf(
			!m_Initialized,
			L"Window [Restore] | Attempted to restore the window before initializing it."
		))
			return;

		if (m_Logger.LogWarningNLIf(
			m_CurrentState == WindowStateType::WINDOWED,
			L"Window [Restore] | Attempted to restore the window when it is already in windowed state."
		))
			return;

		LONG_PTR currentStyle = GetWindowLongPtrW(m_WindowHandle, GWL_STYLE);

		bool failedRetrieve = m_Logger.LogWarningNLIf(
			currentStyle == 0,
			L"Window [Restore] | Failed to retrieve window style."
		);

		if (failedRetrieve)
			return;

		// Bitwise witchery.
		if (currentStyle & WS_POPUP)
		{
			m_Logger.LogInfoNL(L"Window [Restore] | WS_POPUP is set.");

			// If the function succeeds, the return value is the previous value of the specified offset.
			// If the function fails, the return value is zero.To get extended error information, call GetLastError.
			if (SetWindowLongPtrW(m_WindowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW) == 0)
				m_Logger.LogInfoNL(L"Window [Restore] | Failed to set windowed style.");
			else
				m_Logger.LogInfoNL(L"Window [Restore] | Set windowed style.");
		}

		BOOL succeeded = SetWindowPos(
			m_WindowHandle,
			nullptr,
			0,
			0,
			m_WindowSettings.Current.InitialWidth,
			m_WindowSettings.Current.InitialHeight,
			SWP_NOMOVE
		);

		if (!succeeded)
			m_Logger.LogWarningNL(L"Window [Restore] | Failed to set windowed pos.");
		else
			m_Logger.LogInfoNL(L"Window [Restore] | Set windowed pos.");

		ShowWindow(m_WindowHandle, SW_RESTORE);

		m_Logger.LogInfoNL(L"Window [Restore] | Restored window.");

		m_PreviousState = m_CurrentState;
		m_CurrentState = WindowStateType::WINDOWED;
	}

	[[nodiscard]] WindowResolution Window::CurrentResolution() const noexcept
	{
		return WindowResolution { 
			.Width = m_WindowSettings.Current.ClientArea.right,
			.Height = m_WindowSettings.Current.ClientArea.bottom 
		};
	}
#pragma endregion

#pragma region Private
#pragma region State
	void Window::SetShutdownState() noexcept
	{
		m_Running = false;
		m_Logger.LogInfoNL(L"Window [SetShutdownState] | Window shutdown has been requested.");

		Minimize();
	}
#pragma endregion

#pragma region Window Management
	bool Window::Create() noexcept
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

		wndClass.hInstance = mP_Instance;
		wndClass.lpszClassName = S_CLASS_NAME.data();

		RegisterWindowClass(wndClass);

		RECT& clientAreaRef = m_WindowSettings.Current.ClientArea;

		clientAreaRef.right = m_WindowSettings.Current.InitialWidth;
		clientAreaRef.bottom = m_WindowSettings.Current.InitialHeight;

		DWORD dwStyle = m_WindowSettings.Current.UseFullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;

		// Create the window.
		m_WindowHandle = CreateWindowExW(
			0,												 // 0 for no optional window styles.
			wndClass.lpszClassName,							 // Window class.
			m_WindowSettings.Current.WindowTitle.data(),	 // Window title.
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
			 *        as a instance member function so we have access to Window state.
			 */
		);

		m_Logger.LogFatalNLIf(
			m_WindowHandle == nullptr,
			L"Window [Create] | Failed to get a valid window handle. Window handle is nullptr."
		);

		// Here so Intelisense doesn't yell at me for de-referencing a nullptr, even though fatal logs terminate the program.
		if (m_WindowHandle == nullptr)
			return false;

		int showCmd = m_WindowSettings.Current.UseFullscreen ?
			SW_SHOWMAXIMIZED : SW_SHOW;

		//  If nonzero, the return value is nonzero. Otherwise, the window was previously hidden.
		BOOL previouslyVisible = ShowWindow(m_WindowHandle, showCmd);

		BOOL result = GetClientRect(m_WindowHandle, &clientAreaRef);

		m_Logger.LogFatalNLIf(
			!result,
			L"Window [Create] | Failed to update window size."
		);


		m_CurrentState = m_WindowSettings.Current.UseFullscreen ?
			WindowStateType::MAXIMIZED : WindowStateType::WINDOWED;

		m_Running = true;
		return true;
	}

	bool Window::Destroy() noexcept
	{
		if (m_Shutdown)
			return false;

		PostQuitMessage(0);
		m_Running = false;

		m_Logger.LogFatalNLIf(
			DestroyWindow(m_WindowHandle) == 0,
			L"Window [Destroy] | Failed to destroy the window. Please help me."
		);

		m_Logger.LogFatalNLIf(
			UnregisterClassW(S_CLASS_NAME.data(), mP_Instance) == 0,
			L"Window [Destroy] | Failed to unregister the window's class. How did you manage to do this???"
		);

		return true;
	}

	[[nodiscard]] HINSTANCE Window::GetCurrentInstance() noexcept
	{
		HMODULE hModule = nullptr;

		// If the @lpModuleName parameter is NULL, the function returns a
		//   handle to the file used to create the calling process (.exe file).
		BOOL succeeded = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, NULL, &hModule);

		m_Logger.LogFatalNLIf(
			succeeded == 0,
			L"Window [GetCurrentInstance] | Call to GetModuleHandleExW failed."
		);

		m_Logger.LogFatalNLIf(
			hModule == nullptr,
			L"Window [GetCurrentInstance] | Unable to retrieve the current HINSTANCE. Retrieved HINSTANCE was nullptr."
		);

		return static_cast<HINSTANCE>(hModule);
	}

	void Window::RegisterWindowClass(const WNDCLASSEX& wndClassRef) noexcept
	{
		/* https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		 * If the function succeeds, the return value is a class atom that uniquely identifies the class being registered.
		 * This atom can only be used by the CreateWindow, CreateWindowEx, GetClassInfo, GetClassInfoEx, FindWindow,
		 * FindWindowEx, and UnregisterClass functions and the IActiveIMMap::FilterClientWindows method.

		 * If the function fails, the return value is zero. To get extended error information, call GetLastError.
		 */
		m_Logger.LogFatalNLIf(
			!RegisterClassExW(&wndClassRef),
			L"Window [RegisterWindowClass] | Failed to register Window's WNDCLASSEX."
		);
	}
#pragma endregion

#pragma region Settings
	void Window::ValidateSettings() noexcept
	{
		if (m_Logger.LogWarningNLAppendIf(
			m_WindowSettings.Current.WindowTitle.data() == nullptr,
			L"Window [ValidateSettings] | Window title is nullptr. "
			L"Resorting to default : ",WindowData::S_DEFAULT_WINDOW_TITLE.data()
		))
			m_WindowSettings.SetTitleToDefault();

		else if (m_Logger.LogWarningNLAppendIf(
			m_WindowSettings.Current.WindowTitle.size() == 0,
			L"Window [ValidateSettings] | Window title has a character size of 0. "
			L"Resorting to default : ", WindowData::S_DEFAULT_WINDOW_TITLE.data()
		))
			m_WindowSettings.SetTitleToDefault();

		if (m_Logger.LogWarningNLVariadicIf(
			m_WindowSettings.Current.InitialWidth <= 0,
			L"Window [ValidateSettings] | Window width is negative. "
			L"(Width : ", m_WindowSettings.Current.InitialWidth,
			L") Resorting to default : ", WindowData::S_DEFAULT_WIDTH
		))
			m_WindowSettings.SetWidthToDefault();

		else if (m_Logger.LogWarningNLVariadicIf(
			m_WindowSettings.Current.InitialWidth >= m_WindowSettings.MaxWidth,
			L"Window [ValidateSettings] | Window width is greater than max width. "
			L"(Width : ", m_WindowSettings.Current.InitialWidth,
			L" | Max Width : ", m_WindowSettings.MaxWidth,
			L") Resorting to default: ", WindowData::S_DEFAULT_WIDTH
		))
			m_WindowSettings.SetWidthToDefault();

		if (m_Logger.LogWarningNLVariadicIf(
			m_WindowSettings.Current.InitialHeight <= 0,
			L"Window [ValidateSettings] | Window height is negative. "
			L"(Height : ", m_WindowSettings.Current.InitialHeight,
			L") Resorting to default : ", WindowData::S_DEFAULT_HEIGHT
		))
			m_WindowSettings.SetHeightToDefault();

		else if (m_Logger.LogWarningNLVariadicIf(
			m_WindowSettings.Current.InitialHeight >= m_WindowSettings.MaxHeight,
			L"Window [ValidateSettings] | Window height is greater than max width. "
			L"(Height : ", m_WindowSettings.Current.InitialHeight,
			L" | Max Height : ", m_WindowSettings.MaxHeight,
			L") Resorting to default: ", WindowData::S_DEFAULT_HEIGHT
		))
			m_WindowSettings.SetHeightToDefault();
		
		m_WindowSettings.Current.ClientArea = {
			0,
			0,
			m_WindowSettings.Current.InitialWidth,
			m_WindowSettings.Current.InitialHeight
		};
	}

	void Window::LogCurrentSettings() const noexcept
	{
		m_Logger.LogInfoNLAppend(
			L"Window [LogCurrentSettings] | Target Title : ",
			m_WindowSettings.Current.WindowTitle
		);

		m_Logger.LogInfoNLVariadic(
			L"Window [LogCurrentSettings] | Target Window Resolution : ",
			m_WindowSettings.Current.InitialWidth, L" x ", m_WindowSettings.Current.InitialHeight
		);

		m_Logger.LogInfoNLVariadic(
			L"Window [LogCurrentSettings] | Max Window Resolution : ",
			m_WindowSettings.MaxWidth, L" x ", m_WindowSettings.MaxHeight
		);

		m_Logger.LogInfoNLAppend(
			L"Window [LogCurrentSettings] | Use fullscreen : ", 
			Common::Utility::BoolToWStringView(m_WindowSettings.Current.UseFullscreen)
		);
	}
#pragma endregion

#pragma region WndProc
	[[nodiscard]] LRESULT CALLBACK Window::WndProcSetup(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept
	{
		// If we get a message before the WM_NCCREATE message, handle with default window procedure provided by the WinAPI.
		// (WM_NCCREATE contains the instance of Window that was passed to CreateWindowEx)
		if (msgCode != WM_NCCREATE)
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);

		// Retrieve create parameter passed into CreateWindowEx that stores an instance of Window.
		const CREATESTRUCTW* const pCreateStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWindow = static_cast<Window*>(pCreateStruct->lpCreateParams);

		// Set WinAPI-managed user data to store a pointer to Window.
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));

		// Set message procedure to WndProcThunk now that the instance of Window is stored.
		SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Window::WndProcThunk));

		// Forward message to the instance WndProc.
		return pWindow->WndProc(hWnd, msgCode, wParam, lParam);
	}

	[[nodiscard]] LRESULT CALLBACK Window::WndProcThunk(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept
	{
		// Retrieve pointer to Window from WinAPI user data.
		Window* const pWindow = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

		// Forward message to our Window WndProc.
		return pWindow->WndProc(hWnd, msgCode, wParam, lParam);
	}

	[[nodiscard]] LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept
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
				m_Logger.LogInfo(L"Window [WndProc] Window deactivated.\n");
				m_Focused = false;
			}
			else if (LOWORD(wParam) == WA_ACTIVE)
			{
				m_Logger.LogInfo(L"Window [WndProc] Window activated.\n");
				m_Focused = true;
			}

			return S_OK;
		case WM_SYSKEYDOWN:
			// If alt + enter is pressed. ((GetKeyState(VK_MENU) & 0x8000) != 0 alt is held down)
			/*if (wParam == VK_RETURN && (GetKeyState(VK_MENU) & 0x8000) != 0)
				return S_OK;*/

			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		case WM_SYSCOMMAND:
			m_Logger.LogInfo(L"Window [WndProc] | Received SYSCOMMAND.\n");

			if ((wParam & 0xFFF0) == SC_MAXIMIZE)
				Maximize();
			else if ((wParam & 0xFFF0) == SC_MINIMIZE)
				Minimize();
			else if ((wParam & 0xFFF0) == SC_RESTORE)
				Restore();

			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		case WM_SIZE:
			GetClientRect(m_WindowHandle, &m_WindowSettings.Current.ClientArea);
			m_OnResizeFunc();

			[[fallthrough]];
		default:
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		}
	}
#pragma endregion
#pragma endregion
}