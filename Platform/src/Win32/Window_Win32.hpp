#pragma once

#include "IWindow.hpp"
#include "Win32/PlatformFwd_Win32.hpp"

#include <string_view>

namespace Platform::Win32
{
    class Window final : public IWindow
    {
    public:
        Window() noexcept;
        ~Window() noexcept;

        virtual void Update() noexcept override;

        virtual inline [[nodiscard]] bool IsRunning() const noexcept override { return m_IsRunning; }

        inline [[nodiscard]] ::HWND Impl_hWnd() noexcept { return mP_hWnd; }
    private:
        void Init() noexcept;
        void Shutdown() noexcept;

        static [[nodiscard]] ::LRESULT CALLBACK WndProcSetup(
			::HWND hWnd,
			::UINT msgCode,
			::WPARAM wParam,
			::LPARAM lParam
		) noexcept;

		static [[nodiscard]] ::LRESULT CALLBACK WndProcThunk(
			::HWND hWnd,
			::UINT msgCode,
			::WPARAM wParam,
			::LPARAM lParam
		) noexcept;

		[[nodiscard]] ::LRESULT CALLBACK WndProc(
			::HWND hWnd,
			::UINT msgCode,
			::WPARAM wParam,
			::LPARAM lParam
		) noexcept;
    private:
        static constexpr std::wstring_view S_ClassName = L"Window_Win32";
        ::HINSTANCE mP_hInstance = nullptr;
        ::HWND mP_hWnd = nullptr;
        ::RECT m_ClientArea = {};
        ::RECT m_WindowArea = {};
        bool m_IsRunning = false;
    };
}