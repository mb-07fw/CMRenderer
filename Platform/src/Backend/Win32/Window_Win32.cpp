#include "Backend/Win32/Window_Win32.hpp"
#include "Backend/Win32/PlatformOS_Win32.hpp"
#include "IPlatform.hpp"
#include "Common/Assert.hpp"
#include "Common/Cast.hpp"

namespace Platform::Backend::Win32
{
    Window::Window() noexcept
    {
        Init();
        
        std::cout << "Window_Win32!\n";
    }

    Window::~Window() noexcept
    {
        Shutdown();

        std::cout << "Window_Win32 Destroyed!\n";
    }

    void Window::SetWindowResizeCallback(
        void (*pCallback)(uint32_t width, uint32_t height, void* pUserData),
        void* pUserData
    ) noexcept
    {
        m_CurrentResizeCallback.pCallback = pCallback;
        m_CurrentResizeCallback.pUserData = pUserData;
    }

    void Window::ClearWindowResizeCallback() noexcept
    {
        m_CurrentResizeCallback.pCallback = nullptr;
        m_CurrentResizeCallback.pUserData = nullptr;
    }

    void Window::Update() noexcept
    {
        ::BOOL result = 0;
		::MSG msg = {};

		// While there are messages to process. (Return value is non-zero)
		while ((result = ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) != 0)
		{
			if (msg.message == WM_QUIT)
				break;

			// Translate any raw virtual-key messages into character messages.
			::TranslateMessage(&msg);

			// Forward the message to the current window procedure.
			::DispatchMessageW(&msg);
		}
    }

    void Window::Init() noexcept
    {
        ::BOOL succeeded = ::GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            nullptr,
            &mP_hInstance
        );

        ASSERT(succeeded, "(Window_Win32) Failed to retrieve DLL's HINSTANCE.");

        ::WNDCLASSEX wndClass = {};

        wndClass.cbSize = sizeof(wndClass);

        /* Set the window procedure to the static setup function that will eventually thunk (redirect / forward)
         *   Window's messages to an instance function that has access to Window and Renderer state.
         *	  
         *   The function assigned to lpfnWndProc has to be static due to how instance functions work in C++,
         *   and how windows procedures are defined in the WinAPI.
         *	  
         *   For context, Instance functions implicitly take in an argument of 'this', which is incompatible with
         *   the WinAPI window procedure standard, thus static functions are required as they omit the 'this' argument.
         *	  
         *   In conclusion, this indirection is required to be able to access Window state in the WNDPROC without
         *   the introduction of global state.
         */
        wndClass.lpfnWndProc = WndProcSetup;  

		wndClass.hInstance = mP_hInstance;
		wndClass.lpszClassName = S_ClassName.data();

        ASSERT(::RegisterClassExW(&wndClass), "(Window_Win32) Failed to register window class.");

        constexpr long WindowStyle = WS_OVERLAPPEDWINDOW;

        m_ClientArea.right = 800;
		m_ClientArea.bottom = 600;

		m_WindowArea = m_ClientArea;

		/* Adjust window area for desired client area. */
		succeeded = ::AdjustWindowRect(&m_WindowArea, WindowStyle, false);

        ASSERT(succeeded, "(Window_Win32) Failed to adjust to desired client area.");

        mP_hWnd = ::CreateWindowExW(
			0,
			wndClass.lpszClassName,
			wndClass.lpszClassName,
			WindowStyle,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			m_WindowArea.right,
			m_WindowArea.bottom,
			nullptr,
			nullptr,
			wndClass.hInstance,
			this
		);

        ASSERT(mP_hWnd != nullptr, "(Window_Win32) Failed to create the window.");

		::ShowWindow(mP_hWnd, SW_SHOW);
		m_IsRunning = true;
        m_IsVisible = true;
    }

    void Window::Shutdown() noexcept
    {
        ASSERT(::DestroyWindow(mP_hWnd), "(Window_Win32) Failed to destroy window instance.");
        
        ASSERT(::UnregisterClassW(S_ClassName.data(), mP_hInstance), "(Window_Win32) Failed to unregister the window's class.");
    }

    [[nodiscard]] ::LRESULT CALLBACK Window::WndProcSetup(
        ::HWND hWnd,
        ::UINT msgCode,
        ::WPARAM wParam,
        ::LPARAM lParam
    ) noexcept
    {
        /* If we get a message before the WM_NCCREATE message, handle with default window procedure.
		 * (WM_NCCREATE contains the instance of Window that was passed to CreateWindowEx). */
		if (msgCode != WM_NCCREATE)
			return ::DefWindowProcW(hWnd, msgCode, wParam, lParam);

		/* Retrieve create parameter passed into CreateWindowEx that stores an instance of Window. */
		const ::CREATESTRUCTW* const pCreateStruct = Reinterpret<::CREATESTRUCTW*>(lParam);
		Window* const pWindow = Reinterpret<Window*>(pCreateStruct->lpCreateParams);

		/* Set WinAPI-managed user data to store a pointer to this instance of Window. */
		::SetWindowLongPtrW(hWnd, GWLP_USERDATA, Reinterpret<::LONG_PTR>(pWindow));

		/* Set message procedure to WndProcThunk now that the instance of Window is stored. */
		::SetWindowLongPtrW(hWnd, GWLP_WNDPROC, Reinterpret<::LONG_PTR>(Window::WndProcThunk));

		/* Forward message to the instance WndProc. */
		return pWindow->WndProc(hWnd, msgCode, wParam, lParam);
    }

    [[nodiscard]] ::LRESULT CALLBACK Window::WndProcThunk(
        ::HWND hWnd,
        ::UINT msgCode,
        ::WPARAM wParam,
        ::LPARAM lParam
    ) noexcept
    {
        /* Return pointer to Window from WinAPI user data. */
		Window* const pWindow = Reinterpret<Window*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

		/* Forward message to our Window WndProc. */
		return pWindow->WndProc(hWnd, msgCode, wParam, lParam);
    }

    [[nodiscard]] ::LRESULT CALLBACK Window::WndProc(
        ::HWND hWnd,
        ::UINT msgCode,
        ::WPARAM wParam,
        ::LPARAM lParam
    ) noexcept
    {
        switch (msgCode)
        {
        case WM_CLOSE:
			m_IsRunning = false;
			PostQuitMessage(0);
			return 0;
        case WM_SHOWWINDOW:
            m_IsVisible = Cast<bool>(wParam);
            return 0;
        case WM_SIZE:
        {
            /* NOTE: To get the visible window bounds, not including the invisible resize borders,
             *         use DwmGetWindowAttribute, specifying DWMWA_EXTENDED_FRAME_BOUNDS.
             *         (not adjusted for DPI) */
            if (!::GetWindowRect(mP_hWnd, &m_WindowArea))
                LogFatal("(Window_Win32) Failed to retrieve window area after resizing.");

            ::UINT width = LOWORD(lParam);
            ::UINT height = HIWORD(lParam);

            m_ClientArea.right = width;
            m_ClientArea.bottom = height;

            if (width == 0 && height == 0)
                m_IsVisible = false;

            if (auto pCallback = m_CurrentResizeCallback.pCallback)
                pCallback(width, height, m_CurrentResizeCallback.pUserData);
            
            return 0;
        }
        default:
            return DefWindowProcW(hWnd, msgCode, wParam, lParam);
        }
    }
}