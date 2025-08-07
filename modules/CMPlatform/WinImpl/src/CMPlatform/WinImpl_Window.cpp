#include "CMPlatform/WinImpl_PCH.hpp"
#include "CMPlatform/WinImpl_Window.hpp"
#include "CMPlatform/WinImpl_Platform.hpp"

namespace CMEngine::Platform::WinImpl
{
	extern WinImpl_Platform* gP_PlatformInstance;
	extern void WinImpl_Platform_EnforceInstantiated();

	CM_DYNAMIC_LOAD void WinImpl_Window_Update()
	{
		WinImpl_Platform_EnforceInstantiated();

		return gP_PlatformInstance->Impl_Window().Impl_Update();
	}

	CM_DYNAMIC_LOAD bool WinImpl_Window_IsRunning()
	{
		WinImpl_Platform_EnforceInstantiated();

		return gP_PlatformInstance->Impl_Window().Impl_IsRunning();
	}

	CM_DYNAMIC_LOAD bool WinImpl_Window_ShouldClose()
	{
		WinImpl_Platform_EnforceInstantiated();

		return gP_PlatformInstance->Impl_Window().Impl_ShouldClose();
	}

	CM_DYNAMIC_LOAD ScreenResolution WinImpl_Window_Resolution()
	{
		WinImpl_Platform_EnforceInstantiated();

		return gP_PlatformInstance->Impl_Window().Impl_Resolution();
	}

	Window::Window() noexcept
		: IWindow(
			WindowFuncTable(
				WinImpl_Window_Update,
				WinImpl_Window_IsRunning,
				WinImpl_Window_ShouldClose,
				WinImpl_Window_Resolution
		    )
		  )
	{
		Impl_Init();
	}

	Window::~Window() noexcept
	{
		Impl_Shutdown();
	}

	void Window::Impl_Update() noexcept
	{
		BOOL result = 0;
		MSG msg = {};

		// While there are messages to process. (Return value is non-zero)
		while ((result = PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) != 0)
		{
			if (msg.message == WM_QUIT)
				break;

			// Translate any raw virtual-key messages in character messages. (e.g., 'w', 'a', 's', 'd', etc)
			TranslateMessage(&msg);

			// Forward the message to the current window procedure.
			DispatchMessageW(&msg);
		}
	}

	void Window::Impl_SetCallbackOnResize(WindowCallbackSignatureOnResize pCallback, void* pUserData) noexcept
	{
		m_CallbacksOnResize.emplace_back(pCallback, pUserData);
	}

	bool Window::Impl_RemoveCallbackOnResize(WindowCallbackSignatureOnResize pCallback, void* pUserData) noexcept
	{
		for (size_t i = 0; i < m_CallbacksOnResize.size(); ++i)
		{
			WindowCallbackOnResize& currentCallback = m_CallbacksOnResize[i];

			if (currentCallback.pCallback != pCallback && 
				currentCallback.pUserData != pUserData)
				continue;

			/* Utilize std::vector's erase to preserve the order of callbacks after removal. */
			auto whereIt = m_CallbacksOnResize.begin() + i;
			m_CallbacksOnResize.erase(whereIt);

			return true;
		}

		return false;
	}

	[[nodiscard]] ScreenResolution Window::Impl_Resolution() noexcept
	{
		return ScreenResolution{
			Float2(
				static_cast<float>(m_ClientArea.right),
				static_cast<float>(m_ClientArea.bottom)
			)
		};
	}

	void Window::Impl_Init() noexcept
	{
		BOOL succeeded = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, nullptr, &mP_HINSTANCE);

		if (!succeeded)
			spdlog::critical("(WinImpl_Window) Internal error: Failed to retrieve DLL's HINSTANCE.");

		WNDCLASSEX wndClass = {};

		wndClass.cbSize = sizeof(decltype(wndClass));
		wndClass.lpfnWndProc = WndProcSetup;  /* Set the window procedure to the static setup function that will eventually thunk (redirect / forward)
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
		wndClass.hInstance = mP_HINSTANCE;
		wndClass.lpszClassName = S_CLASS_NAME.data();

		if (!RegisterClassEx(&wndClass))
			spdlog::critical("(WinImpl_Window) Internal error: Failed to register window class.");

		constexpr long WindowStyle = WS_OVERLAPPEDWINDOW;
		
		/* TODO: Keep separate variables for client size and actual window size. */
		m_ClientArea.right = 800;
		m_ClientArea.bottom = 600;

		succeeded = AdjustWindowRect(&m_ClientArea, WindowStyle, false);

		if (!succeeded)
			spdlog::critical("(WinImpl_Window) Internal error: Failed to adjust to desired client area.");

		mP_HWND = CreateWindowEx(
			0,
			wndClass.lpszClassName,
			wndClass.lpszClassName,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			m_ClientArea.right,
			m_ClientArea.bottom,
			nullptr,
			nullptr,
			wndClass.hInstance,
			this
		);

		if (mP_HWND == nullptr)
		{
			spdlog::critical("(WinImpl_Window) Internal error: Failed to create the window.");
			std::exit(-1);
		}

		ShowWindow(mP_HWND, SW_SHOW);

		m_Running = true;

		spdlog::info("(WinImpl_Window) Window Init!");
	}

	void Window::Impl_Shutdown() noexcept
	{
		if (!DestroyWindow(mP_HWND))
			spdlog::critical("(WinImpl_Window) Internal error: Failed to destroy window instance.");

		if (!UnregisterClass(S_CLASS_NAME.data(), mP_HINSTANCE))
			spdlog::critical("(WinImpl_Window) Internal error: Failed to unregister the window's class.");

		spdlog::info("(WinImpl_Window) Window Shutdown!");
	}

	void Window::Impl_NotifyOnResize() noexcept
	{
		for (const WindowCallbackOnResize& callback : m_CallbacksOnResize)
			callback.pCallback(Impl_Resolution(), callback.pUserData);
	}

	[[nodiscard]] LRESULT CALLBACK Window::WndProcSetup(
		HWND hWnd,
		UINT msgCode,
		WPARAM wParam,
		LPARAM lParam
	) noexcept
	{
		/* If we get a message before the WM_NCCREATE message, handle with default window procedure.
		 * (WM_NCCREATE contains the instance of Window that was passed to CreateWindowEx). */
		if (msgCode != WM_NCCREATE)
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);

		/* Retrieve create parameter passed into CreateWindowEx that stores an instance of Window. */
		const CREATESTRUCTW* const pCreateStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWindow = static_cast<Window*>(pCreateStruct->lpCreateParams);

		/* Set WinAPI-managed user data to store a pointer to this instance of Window. */
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));

		/* Set message procedure to WndProcThunk now that the instance of Window is stored. */
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Window::WndProcThunk));

		/* Forward message to the instance WndProc. */
		return pWindow->WndProc(hWnd, msgCode, wParam, lParam);
	}

	[[nodiscard]] LRESULT CALLBACK Window::WndProcThunk(
		HWND hWnd,
		UINT msgCode,
		WPARAM wParam,
		LPARAM lParam
	) noexcept
	{
		/* Return pointer to Window from WinAPI user data. */
		Window* const pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		/* Forward message to our Window WndProc. */
		return pWindow->WndProc(hWnd, msgCode, wParam, lParam);
	}

	[[nodiscard]] LRESULT CALLBACK Window::WndProc(
		HWND hWnd,
		UINT msgCode,
		WPARAM wParam,
		LPARAM lParam
	) noexcept
	{
		switch (msgCode)
		{
		case WM_KEYUP:
			if (wParam != VK_ESCAPE)
				return DefWindowProcW(hWnd, msgCode, wParam, lParam);

			[[fallthrough]];
		case WM_CLOSE:
			m_Running = false;
			PostQuitMessage(0);
			return S_OK;
		case WM_SIZE:
			if (!GetClientRect(mP_HWND, &m_ClientArea))
				spdlog::critical("(WinImpl_Window) Internal error: Failed to retrieve window client area after resizing.");

			Impl_NotifyOnResize();
			return S_OK;
		default:
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		}
	}
}