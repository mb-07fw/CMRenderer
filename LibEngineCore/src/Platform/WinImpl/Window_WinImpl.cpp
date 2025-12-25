#include "PCH.hpp"
#include "Platform/WinImpl/Window_WinImpl.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace CMEngine::Platform::WinImpl
{
	constexpr [[nodiscard]] KeycodeType TranslateVirtualKey(WPARAM vk)
	{
		if (vk >= 'A' && vk <= 'Z')
			return static_cast<KeycodeType>(
				(uint8_t)KeycodeType::A + (vk - 'A')
			);

		switch (vk)
		{
		case VK_F1:      return KeycodeType::F1;
		case VK_RETURN:  return KeycodeType::Return;
		case VK_BACK:    return KeycodeType::Back;
		case VK_MENU:    return KeycodeType::Alt;
		case VK_SHIFT:   return KeycodeType::Shift;
		case VK_CONTROL: return KeycodeType::Ctrl;
		case VK_SPACE:   return KeycodeType::Space;
		default:		 return KeycodeType::Unknown;
		}
	}

	Window::Window(Event::EventSystem& eventSystem) noexcept
		: m_EventSystem(eventSystem)
	{
		Init();

		m_PressedKeys.reserve((size_t)KeycodeType::Total_Values);
	}

	Window::~Window() noexcept
	{
		Shutdown();
	}

	void Window::Update() noexcept
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

	void Window::SetCallbackOnResize(WindowCallbackSignatureOnResize pCallback, void* pUserData) noexcept
	{
		m_CallbacksOnResize.emplace_back(pCallback, pUserData);
	}

	bool Window::RemoveCallbackOnResize(WindowCallbackSignatureOnResize pCallback, void* pUserData) noexcept
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

	void Window::Init() noexcept
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
		
		m_ClientArea.right = 800;
		m_ClientArea.bottom = 600;

		m_WindowArea = m_ClientArea;

		/* Adjust window area for desired client area. */
		succeeded = AdjustWindowRect(&m_WindowArea, WindowStyle, false);

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

	void Window::Shutdown() noexcept
	{
		if (!DestroyWindow(mP_HWND))
			spdlog::critical("(WinImpl_Window) Internal error: Failed to destroy window instance.");

		if (!UnregisterClass(S_CLASS_NAME.data(), mP_HINSTANCE))
			spdlog::critical("(WinImpl_Window) Internal error: Failed to unregister the window's class.");

		spdlog::info("(WinImpl_Window) Window Shutdown!");
	}

	void Window::NotifyOnResize() noexcept
	{
		for (const WindowCallbackOnResize& callback : m_CallbacksOnResize)
			callback.pCallback(ClientResolution(), callback.pUserData);
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
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msgCode, wParam, lParam))
			return true;

		switch (msgCode)
		{
		case WM_KEYDOWN:
		{
			KeycodeType keycode = TranslateVirtualKey(wParam);

			/* Bit 30 is set if the message is a repeat. */
			bool isRepeat = (lParam & (1 << 30)) != 0;
			if (isRepeat)
				return S_OK;

			Event::KeyPressed event(keycode);
			m_EventSystem.Dispatch(event);

			m_PressedKeys.emplace_back(keycode);
			return S_OK;
		}
		case WM_KEYUP:
			if (wParam != VK_ESCAPE)
			{
				KeycodeType keycode = TranslateVirtualKey(wParam);

				Event::KeyReleased event(keycode);
				m_EventSystem.Dispatch(event);

				/* TODO: Come up with a more efficient approach for this... */
				if (m_PressedKeys.size() != 1)
					for (size_t i = 0; i < m_PressedKeys.size(); ++i)
					{
						KeycodeType pressedKeycode = m_PressedKeys[i];

						if (keycode == pressedKeycode)
						{
							m_PressedKeys[i] = m_PressedKeys.back();
							break;
						}
					}

				m_PressedKeys.pop_back();
				return S_OK;
			}
			
			/* Close if escape is pressed... */
			[[fallthrough]];
		case WM_CLOSE:
			m_Running = false;
			PostQuitMessage(0);
			return S_OK;
		case WM_KILLFOCUS: [[fallthrough]];
		case WM_ACTIVATEAPP: // wParam == FALSE means deactivated
		{
			/* Guarded, since events shouldn't be dispatched when the program window is closing. */
			if (m_Running)
			{
				/* Release any key's that were active when the window loses focus... */
				Event::KeyReleased event;
				for (KeycodeType keycode : m_PressedKeys)
				{
					event.Keycode = keycode;
					m_EventSystem.Dispatch(event);
				}

				m_PressedKeys.clear();
			}
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		}
		case WM_SIZE:
			if (!GetClientRect(mP_HWND, &m_ClientArea))
				spdlog::critical("(WinImpl_Window) Internal error: Failed to retrieve client area after resizing.");
			/* NOTE: To get the visible window bounds, not including the invisible resize borders,
			 *         use DwmGetWindowAttribute, specifying DWMWA_EXTENDED_FRAME_BOUNDS. Unlike the
			 *         Window Rect, the DWM Extended Frame Bounds are not adjusted for DPI. */
			else if (!GetWindowRect(mP_HWND, &m_WindowArea))
				spdlog::critical("(WinImpl_Window) Internal error: Failed to retrieve window area after resizing.");

			NotifyOnResize();
			return S_OK;
		default:
			return DefWindowProcW(hWnd, msgCode, wParam, lParam);
		}
	}
}