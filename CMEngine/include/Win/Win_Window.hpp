#pragma once

#include <minwindef.h>

#include <functional>
#include <string_view>

#include "Win/Win_WindowSettings.hpp"
#include "Win/Win_Keyboard.hpp"
#include "Common/Logger.hpp"

namespace CMEngine::Win
{
	enum class WindowStateType : uint8_t
	{
		INVALID,
		MAXIMIZED,
		MINIMIZED,
		WINDOWED
	};

	class Window
	{
	public:
		Window(
			Common::LoggerWide& logger,
			WindowSettings& windowSettings,
			std::function<void()> onResizeFunc
		) noexcept;

		~Window() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;

		void HandleMessages() noexcept;
		
		/* @handleMillis: Time to handle messages before returning once an initial message is received. */
		void WaitForMessages(long long handleMillis) noexcept;

		void Maximize() noexcept;
		void Minimize() noexcept;
		void Restore() noexcept;

		[[nodiscard]] WindowResolution CurrentResolution() const noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
		inline [[nodiscard]] bool IsRunning() const noexcept { return m_Running; }
		
		inline [[nodiscard]] bool IsMaximized() const noexcept { return m_CurrentState == WindowStateType::MAXIMIZED; }
		inline [[nodiscard]] bool IsMinimized() const noexcept { return m_CurrentState == WindowStateType::MINIMIZED; }
		inline [[nodiscard]] bool IsWindowed() const noexcept { return m_CurrentState == WindowStateType::WINDOWED; }
		inline [[nodiscard]] WindowStateType WindowState() const noexcept { return m_CurrentState; }

		inline [[nodiscard]] bool IsFocused() const noexcept { return m_Focused; }

		inline [[nodiscard]] const HWND Handle() const noexcept { return m_WindowHandle; }
		inline [[nodiscard]] RECT ClientArea() const noexcept { return m_WindowSettings.Current.ClientArea; }
		inline [[nodiscard]] Keyboard& Keyboard() noexcept { return m_Keyboard; }
	private:
		void SetShutdownState() noexcept;

		bool Create() noexcept;
		bool Destroy() noexcept;

		[[nodiscard]] HINSTANCE GetCurrentInstance() noexcept;
		void RegisterWindowClass(const WNDCLASSEX& wndClassRef) noexcept;

		void ValidateSettings() noexcept;
		void LogCurrentSettings() const noexcept;

		static [[nodiscard]] LRESULT CALLBACK WndProcSetup(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept;
		static [[nodiscard]] LRESULT CALLBACK WndProcThunk(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept;

		[[nodiscard]] LRESULT CALLBACK WndProc(HWND hWnd, UINT msgCode, WPARAM wParam, LPARAM lParam) noexcept;
	private:
		static constexpr std::wstring_view S_CLASS_NAME = L"Window";
		Common::LoggerWide& m_Logger;
		WindowSettings& m_WindowSettings;
		std::function<void()> m_OnResizeFunc;
		HINSTANCE mP_Instance = nullptr;
		HWND m_WindowHandle = nullptr;
		Win::Keyboard m_Keyboard;
		WindowStateType m_CurrentState = WindowStateType::INVALID;
		WindowStateType m_PreviousState = WindowStateType::INVALID;
		bool m_Running = false;
		bool m_Focused = false;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}