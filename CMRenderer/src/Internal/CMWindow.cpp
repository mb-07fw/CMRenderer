#include "Core/CMPCH.hpp"
#include "Internal/CMWindow.hpp"

#include "Core/Utility.hpp"

namespace CMRenderer
{
#pragma region Public
	CMWindow::CMWindow(CMRendererSettings& cmSettingsRef, CMLoggerWide& cmLoggerRef) noexcept
		: m_CMSettingsRef(cmSettingsRef), m_WindowSettingsRef(cmSettingsRef.WindowSettings),
		  m_CMLoggerRef(cmLoggerRef)
	{
		ValidateSettings();

		m_hInstance = GetCurrentHINSTANCE();

		LogCurrentSettings();
	}

	void CMWindow::LogCurrentSettings() const noexcept
	{
		std::wstring message = L"CMWindow [LogCurrentSettings] | Target Title : " + std::wstring(m_WindowSettingsRef.Current.WindowTitle) + L'\n';

		m_CMLoggerRef.LogInfo(message);

		message = L"CMWindow [LogCurrentSettings] | Target Window Resolution : " +
			std::to_wstring(m_WindowSettingsRef.Current.Width) + L" x " + std::to_wstring(m_WindowSettingsRef.Current.Height) + L'\n';

		m_CMLoggerRef.LogInfo(message);

		message = L"CMWindow [LogCurrentSettings] | Max Window Resolution : " +
			std::to_wstring(m_WindowSettingsRef.MaxWidth) + L" x " + std::to_wstring(m_WindowSettingsRef.MaxHeight) + L'\n';

		m_CMLoggerRef.LogInfo(message);

		message = L"CMWindow [LogCurrentSettings] | Use fullscreen : " + Utility::BoolToWStr(m_WindowSettingsRef.Current.UseFullscreen) + L'\n';

		m_CMLoggerRef.LogInfo(message);
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
			m_CMLoggerRef.LogWarning(L"CMWindow [Init] | Initialization has been attempted after CMWindow has already been initialized.\n");
			return;
		};

		bool createdWindow = Create();

		if (!createdWindow)
			m_CMLoggerRef.LogFatal(L"CMWindow [Init] | Failed to create the window.\n");

		m_Initialized = true;
		m_Shutdown = false;
	}

	void CMWindow::Shutdown() noexcept
	{
		if (m_Shutdown)
		{
			m_CMLoggerRef.LogWarning(L"CMWindow [Shutdown] | Shutdown has been attempted after CMWindow has already been shutdown.\n");
			return;
		}
		else if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarning(L"CMWindow [Shutdown] | Shutdown has been attempted before CMWindow has been initialized.\n");
			return;
		}

		bool destroyed = Destroy();

		if (!destroyed)
			m_CMLoggerRef.LogFatal(L"CMWindow [Shutdown] | Failed to destroy the window.\n");

		m_Initialized = false;
		m_Shutdown = true;

		m_CMLoggerRef.LogInfo(L"CMWindow [Shutdown] | Shutdown.\n");
	}

	void CMWindow::HandleMessages() noexcept
	{
		BOOL result = 0;
		MSG msg;

		// While there are messages to process. (Return value is non-zero)
		while ((result = PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) > 0)
		{
			if (msg.message == WM_QUIT)
			{
				m_Running = false;

				m_CMLoggerRef.LogInfo(L"CMWindow [HandleMessages] | Window shutdown has been requested.\n");

				return;
			}

			// Translate any raw virtual-key messages in character messages. (e.g., 'w', 'a', 's', 'd', etc)
			TranslateMessage(&msg);

			// Forward the message to the current window procedure.
			DispatchMessageW(&msg);
		}
	}

	void CMWindow::Minimize() const noexcept
	{
		if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarning(L"CMWindow [Minimize] | Attempted to minimize the window before initializing it.\n");
			return;
		}

		ShowWindow(m_WindowHandle, SW_SHOWMINIMIZED);
	}
#pragma endregion

#pragma region Private
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

		DWORD dwStyle = m_WindowSettingsRef.Current.UseFullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW | WS_SYSMENU;

		// Create the window.
		m_WindowHandle = CreateWindowExW(
			0,														// 0 for no optional window styles.
			wndClass.lpszClassName,									// Window class.
			m_WindowSettingsRef.Current.WindowTitle.data(), // Window title.
			dwStyle,												// Window styles. 
			CW_USEDEFAULT, CW_USEDEFAULT,							// X,Y position.
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
			m_CMLoggerRef.LogFatal(L"CMWindow [Create] | Failed to get a valid window handle. Window handle is nullptr.\n");
			return false;
		}

		int showCmd = m_WindowSettingsRef.Current.UseFullscreen ? SW_SHOWMAXIMIZED : SW_SHOW;

		// If nonzero, the return value is nonzero. Otherwise, the window was previously hidden.
		BOOL previouslyVisible = ShowWindow(m_WindowHandle, showCmd);

		BOOL result = GetClientRect(m_WindowHandle, &m_ClientArea);

		if (result == 0)
		{
			m_CMLoggerRef.LogFatal(L"CMWindow [Create] | Failed to update window size.\n");
			return false;
		}

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
			m_CMLoggerRef.LogWarning(L"CMWindow [Destroy] | Failed to destroy the window handle : ");
			
			return false;
		}

		int unregisteredClass = UnregisterClassW(S_CLASS_NAME.data(), m_hInstance);

		if (unregisteredClass == 0)
		{
			m_CMLoggerRef.LogWarning(L"CMWindow [Destroy] | Failed to unregisted the window's class. How did you manage to do this???\n");
			return false;
		}

		return true;
	}

	void CMWindow::ValidateSettings() noexcept
	{
		if (m_WindowSettingsRef.Current.WindowTitle.data() == nullptr)
		{
			std::wstring message = L"CMWindow [ValidateSettings] | Window title is nullptr. Resorting to default : " +
				std::wstring(CMWindowData::S_DEFAULT_WINDOW_TITLE.data()) + L'\n';
			m_CMLoggerRef.LogWarning(message);
			m_WindowSettingsRef.SetTitleToDefault();
		}

		else if (m_WindowSettingsRef.Current.WindowTitle.size() == 0)
		{
			std::wstring message = L"CMWindow [ValidateSettings] | Window title has a size of 0. Resorting to default : " +
				std::wstring(CMWindowData::S_DEFAULT_WINDOW_TITLE.data()) + L'\n';

			m_CMLoggerRef.LogWarning(message);
			m_WindowSettingsRef.SetTitleToDefault();
		}

		if (m_WindowSettingsRef.Current.Width <= 0)
		{
			std::wstring message = L"CMWindow [ValidateSettings] | Window width is negative. (Width : " +
				std::to_wstring(m_WindowSettingsRef.Current.Width) + L") Resorting to default : " +
				std::to_wstring(CMWindowData::S_DEFAULT_WIDTH) + L'\n';

			m_CMLoggerRef.LogWarning(message);
			m_WindowSettingsRef.SetWidthToDefault();
		}

		else if (m_WindowSettingsRef.Current.Width >= m_WindowSettingsRef.MaxWidth)
		{
			std::wstring message = L"CMWindow [ValidateSettings] | Window width is greater than max width. (Width : " +
				std::to_wstring(m_WindowSettingsRef.Current.Width) + L" | Max Width : " +
				std::to_wstring(m_WindowSettingsRef.MaxWidth) + L") Resorting to default: " +
				std::to_wstring(CMWindowData::S_DEFAULT_WIDTH) + L'\n';

			m_CMLoggerRef.LogWarning(message);
			m_WindowSettingsRef.SetWidthToDefault();
		}

		if (m_WindowSettingsRef.Current.Height <= 0)
		{
			std::wstring message = L"CMWindow [ValidateSettings] | Window height is negative. (Height : " +
				std::to_wstring(m_WindowSettingsRef.Current.Height) + L") Resorting to default : " +
				std::to_wstring(CMWindowData::S_DEFAULT_HEIGHT) + L'\n';

			m_CMLoggerRef.LogWarning(message);
			m_WindowSettingsRef.SetHeightToDefault();
		}

		else if (m_WindowSettingsRef.Current.Height >= m_WindowSettingsRef.MaxHeight)
		{
			std::wstring message = L"CMWindow [ValidateSettings] | Window height is greater than max width. (Height : " +
				std::to_wstring(m_WindowSettingsRef.Current.Height) + L" | Max Height : " +
				std::to_wstring(m_WindowSettingsRef.MaxHeight) + L") Resorting to default: " +
				std::to_wstring(CMWindowData::S_DEFAULT_HEIGHT) + L'\n';

			m_CMLoggerRef.LogWarning(message);
			m_WindowSettingsRef.SetHeightToDefault();
		}
	}

	[[nodiscard]] HINSTANCE CMWindow::GetCurrentHINSTANCE() noexcept
	{
		HMODULE hModule = nullptr;
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, NULL, &hModule);

		if (hModule == nullptr)
			m_CMLoggerRef.LogFatal(L"CMWindow [GetCurrentHINSTANCE] | Unable to retrieve the current HINSTANCE. Retrieved HINSTANCE was nullptr.\n");

		return static_cast<HINSTANCE>(hModule);
	}

	void CMWindow::RegisterWindowClass(const WNDCLASSEX& wndClassRef) noexcept
	{
		bool registeredClass = RegisterClassExW(&wndClassRef);

		if (!registeredClass)
			m_CMLoggerRef.LogFatal(L"CMWindow [RegisterWindowClass] | Failed to register CMWindow's WNDCLASSEX.\n");
	}

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
				return DefWindowProcW(hWnd, msgCode, wParam, lParam);

			[[fallthrough]];
		case WM_CLOSE:
			PostQuitMessage(0);
			return S_OK;
		default:
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		}
	}
#pragma endregion
#pragma endregion
}