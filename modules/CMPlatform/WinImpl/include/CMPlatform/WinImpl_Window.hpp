#pragma once

#include "CMPlatform/IWindow.hpp"
#include "CMPlatform/Export.hpp"

#include <string_view>
#include <vector>

namespace CMEngine::Platform::WinImpl
{
	struct WindowAction
	{
		/* Unscoped enum is used for `extern "C"` compatibility. */
		enum Type : int8_t
		{
			INVALID = -1,
			RESIZE,
			TOTAL_ACTIONS
		};
	};

	class Window : public IWindow
	{
	public:
		Window() noexcept;
		~Window() noexcept;

		Window(const Window& other) = delete;
		Window operator=(const Window& other) = delete;
	public:
		void Impl_Update() noexcept;

		/* WARNING: The lifetime of @pUserData is not managed at all by Window. */
		void Impl_SetCallbackOnResize(WindowCallbackSignatureOnResize pCallback, void* pUserData) noexcept;

		/* Removes the registered callback.
		 * Returns true if the callback was present and removed (as identified by both pointers); false otherwise. */
		bool Impl_RemoveCallbackOnResize(WindowCallbackSignatureOnResize pCallback, void* pUserData) noexcept;

		inline [[nodiscard]] bool Impl_IsRunning() const noexcept { return m_Running; }
		inline [[nodiscard]] bool Impl_ShouldClose() const noexcept { return !m_Running; }
		inline [[nodiscard]] HWND Impl_HWND() noexcept { return mP_HWND; }
		[[nodiscard]] ScreenResolution Impl_Resolution() noexcept;
	private:
		void Impl_Init() noexcept;
		void Impl_Shutdown() noexcept;

		void Impl_NotifyOnResize() noexcept;

		static [[nodiscard]] LRESULT CALLBACK WndProcSetup(
			HWND hWnd,
			UINT msgCode,
			WPARAM wParam,
			LPARAM lParam
		) noexcept;

		static [[nodiscard]] LRESULT CALLBACK WndProcThunk(
			HWND hWnd,
			UINT msgCode,
			WPARAM wParam,
			LPARAM lParam
		) noexcept;

		[[nodiscard]] LRESULT CALLBACK WndProc(
			HWND hWnd,
			UINT msgCode,
			WPARAM wParam,
			LPARAM lParam
		) noexcept;
	private:
		static constexpr std::wstring_view S_CLASS_NAME = L"WinImpl_Window";
		HINSTANCE mP_HINSTANCE = nullptr;
		HWND mP_HWND = nullptr;
		bool m_Running = false;
		RECT m_ClientArea = {};
		std::vector<WindowCallbackOnResize> m_CallbacksOnResize;
	};

	CM_DYNAMIC_LOAD void WinImpl_Window_Update();
	CM_DYNAMIC_LOAD bool WinImpl_Window_IsRunning();
	CM_DYNAMIC_LOAD bool WinImpl_Window_ShouldClose();
	CM_DYNAMIC_LOAD ScreenResolution WinImpl_Window_Resolution();
}