#pragma once

#include "Platform/IWindow.hpp"
#include "Platform/Export.hpp"

#include <string_view>

namespace CMEngine::Platform::WinImpl
{
	class Window : public IWindow
	{
	public:
		Window() noexcept;
		~Window() noexcept;

		Window(const Window& other) = delete;
		Window operator=(const Window& other) = delete;
	public:
		void Impl_Update() noexcept;

		inline [[nodiscard]] bool Impl_IsRunning() const noexcept { return m_Running; }
		inline [[nodiscard]] bool Impl_ShouldClose() const noexcept { return !m_Running; }
		inline [[nodiscard]] HWND Impl_HWND() noexcept { return mP_HWND; }
	private:
		void Impl_Init() noexcept;
		void Impl_Shutdown() noexcept;

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
	};

	CM_DYNAMIC_LOAD bool WinImpl_Window_IsRunning();
	CM_DYNAMIC_LOAD bool WinImpl_Window_ShouldClose();
}