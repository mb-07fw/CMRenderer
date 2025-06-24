#pragma once

#include "Windows/CME_WNKeyboard.hpp"
#include "Core/CME_WindowSettings.hpp"
#include "CMC_Logger.hpp"

#include <minwindef.h> // Include Windows type definitions.

#include <functional>
#include <string_view>

namespace CMEngine::WindowsAPI
{
	enum class CMWindowStateType : uint8_t
	{
		INVALID,
		MAXIMIZED,
		MINIMIZED,
		WINDOWED
	};

	class CMWindow
	{
	public:
		CMWindow(
			CMCommon::CMLoggerWide& logger,
			CMWindowSettings& windowSettings,
			std::function<void()> onResizeFunc
		) noexcept;

		~CMWindow() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;

		void HandleMessages() noexcept;
		
		/* @handleMillis: Time to handle messages before returning once an initial message is received. */
		void WaitForMessages(long long handleMillis) noexcept;

		void Maximize() noexcept;
		void Minimize() noexcept;
		void Restore() noexcept;

		[[nodiscard]] CMWindowResolution CurrentResolution() const noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
		inline [[nodiscard]] bool IsRunning() const noexcept { return m_Running; }
		
		inline [[nodiscard]] bool IsMaximized() const noexcept { return m_CurrentState == CMWindowStateType::MAXIMIZED; }
		inline [[nodiscard]] bool IsMinimized() const noexcept { return m_CurrentState == CMWindowStateType::MINIMIZED; }
		inline [[nodiscard]] bool IsWindowed() const noexcept { return m_CurrentState == CMWindowStateType::WINDOWED; }

		inline [[nodiscard]] bool IsFocused() const noexcept { return m_Focused; }

		inline [[nodiscard]] const HWND Handle() const noexcept { return m_WindowHandle; }
		inline [[nodiscard]] RECT ClientArea() const noexcept { return m_WindowSettings.Current.ClientArea; }
		inline [[nodiscard]] CMKeyboard& Keyboard() noexcept { return m_Keyboard; }
	private:
		void SetShutdownState() noexcept;

		bool Create() noexcept;
		bool Destroy() noexcept;

		[[nodiscard]] HINSTANCE GetCurrentHINSTANCE() noexcept;
		void RegisterWindowClass(const WNDCLASSEX& wndClassRef) noexcept;

		void ValidateSettings() noexcept;
		void LogCurrentSettings() const noexcept;

		static [[nodiscard]] LRESULT CALLBACK WndProcSetup(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept;
		static [[nodiscard]] LRESULT CALLBACK WndProcThunk(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept;

		[[nodiscard]] LRESULT CALLBACK WndProc(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept;
	private:
		static constexpr std::wstring_view S_CLASS_NAME = L"CMWindow";
		CMCommon::CMLoggerWide& m_Logger;
		CMWindowSettings& m_WindowSettings;
		std::function<void()> m_OnResizeFunc;
		HINSTANCE m_hInstance = nullptr;
		HWND m_WindowHandle = nullptr;
		CMKeyboard m_Keyboard;
		CMWindowStateType m_CurrentState = CMWindowStateType::INVALID;
		CMWindowStateType m_PreviousState = CMWindowStateType::INVALID;
		bool m_Running = false;
		bool m_Focused = false;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}