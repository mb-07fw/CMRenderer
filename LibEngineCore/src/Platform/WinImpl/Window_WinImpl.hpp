#pragma once

#include "Platform/Core/IWindow.hpp"
#include "Platform/WinImpl/PlatformOSFwd_WinImpl.hpp"

#include <string_view>
#include <vector>

namespace CMEngine::Platform::WinImpl
{
	inline constexpr [[nodiscard]] Rect RECTToRect(RECT) noexcept;
	inline constexpr [[nodiscard]] Float2 RECTToFloat2(RECT) noexcept;

	class Window : public IWindow
	{
	public:
		Window() noexcept;
		~Window() noexcept;

		Window(const Window& other) = delete;
		Window operator=(const Window& other) = delete;
	public:
		virtual void Update() noexcept override;

		/* WARNING: The lifetime of @pUserData is not managed at all by Window. */
		void SetCallbackOnResize(WindowCallbackSignatureOnResize pCallback, void* pUserData) noexcept;

		/* Removes the registered callback.
		 * Returns true if the callback was present and removed (as identified by both pointers); false otherwise. */
		bool RemoveCallbackOnResize(WindowCallbackSignatureOnResize pCallback, void* pUserData) noexcept;

		inline [[nodiscard]] HWND Impl_HWND() noexcept { return mP_HWND; }

		/* Returns the width (x) and height (y) of the window's current client area. */
		inline [[nodiscard]] Float2 ClientResolution() const noexcept { return RECTToFloat2(m_ClientArea); }

		/* Returns the width (x) and height (y) of the window's current area (bounding box). */
		inline [[nodiscard]] Float2 WindowResolution() const noexcept { return RECTToFloat2(m_WindowArea); }

		/* Returns the bounding box of the window's current client area (`left` and `top` are 0.0f). */
		inline [[nodiscard]] Rect ClientArea() const noexcept { return RECTToRect(m_ClientArea); }

		/* Returns the bounding box of the window's current area (`left` and `top` are 0.0f). */
		inline [[nodiscard]] Rect WindowArea() const noexcept { return RECTToRect(m_WindowArea); }

		inline virtual [[nodiscard]] bool IsRunning() const noexcept override { return m_Running; }
		inline virtual [[nodiscard]] bool ShouldClose() const noexcept override { return !m_Running; }
	private:
		void Init() noexcept;
		void Shutdown() noexcept;

		void NotifyOnResize() noexcept;

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
		RECT m_WindowArea = {};
		std::vector<WindowCallbackOnResize> m_CallbacksOnResize;
	};

	inline constexpr [[nodiscard]] Rect RECTToRect(RECT rect) noexcept
	{
		return Rect(
			static_cast<float>(rect.left),
			static_cast<float>(rect.top),
			static_cast<float>(rect.right),
			static_cast<float>(rect.bottom)
		);
	}

	inline constexpr [[nodiscard]] Float2 RECTToFloat2(RECT rect) noexcept
	{
		return Float2(
			static_cast<float>(rect.right),
			static_cast<float>(rect.bottom)
		);
	}
}