#pragma once

#include "Internal/CMRendererSettings.hpp"
#include "Internal/CMLogger.hpp"

#include <minwindef.h> // Include Windows type definitions.

#include <functional>
#include <string>
#include <string_view>

namespace CMRenderer
{
	/*enum class CMDisplayType
	{
		INVALID = -1,
		MINIMIZED,
		MAXIMIZED,
		WINDOWED
	};

	struct CMWindowDisplayState
	{
		void SetState(CMDisplayType state) noexcept;
		void RestorePrevious() noexcept;

		[[nodiscard]] std::wstring_view CurrentStateToWStrView() const noexcept;
		[[nodiscard]] std::wstring_view PreviousStateToWStrView() const noexcept;

		[[nodiscard]] std::wstring CurrentStateToWStr() const noexcept;
		[[nodiscard]] std::wstring PreviousStateToWStr() const noexcept;
		
		[[nodiscard]] bool WasFullscreen() noexcept;
		[[nodiscard]] bool WasWindowed() noexcept;
		[[nodiscard]] bool WasMinimized() noexcept;
		[[nodiscard]] bool WasResized() noexcept;

		inline static constexpr [[nodiscard]] std::wstring_view DisplayStateToWStrView(CMDisplayType type) noexcept;

		CMDisplayType CurrentState = CMDisplayType::INVALID;
		CMDisplayType PreviousState = CMDisplayType::INVALID;

		bool IsFullscreen = false;
		bool IsWindowed = false;
		bool IsMinimized = false;

		bool HasResized = false;
	private:
		bool m_WasFullscreen = false;
		bool m_WasWindowed = false;
		bool m_WasMinimized = false;
	};

	inline constexpr [[nodiscard]] std::wstring_view CMWindowDisplayState::DisplayStateToWStrView(CMDisplayType type) noexcept
	{
		switch (type)
		{
		case CMDisplayType::INVALID:	return std::wstring_view(L"INVALID");
		case CMDisplayType::MINIMIZED:  return std::wstring_view(L"MINIMIZED");
		case CMDisplayType::MAXIMIZED:  return std::wstring_view(L"MAXIMIZED");
		case CMDisplayType::WINDOWED:   return std::wstring_view(L"WINDOWED");
		default:						return std::wstring_view(L"UNKNOWN");
		}
	}*/



	class CMWindow
	{
	public:
		CMWindow(CMRendererSettings& cmSettingsRef, CMLoggerWide& cmLoggerRef) noexcept;
		~CMWindow() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;

		void HandleMessages() noexcept;
		
		// handleMillis : Time to handle messages before returning once an initial message is received.
		void WaitForMessages(long long handleMillis) noexcept;

		void Maximize() noexcept;
		void Minimize() noexcept;
		void Restore() noexcept;

		void SetCharKeyCallback(char c, std::function<void(bool)> func) noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
		inline [[nodiscard]] bool IsRunning() const noexcept { return m_Running; }
		
		inline [[nodiscard]] bool IsMaximized() const noexcept { return m_Maximized; }
		inline [[nodiscard]] bool IsMinimized() const noexcept { return m_Minimized; }
		inline [[nodiscard]] bool IsWindowed() const noexcept { return m_Windowed; }

		/*inline [[nodiscard]] bool WasFullscreen() noexcept { return m_DisplayState.WasFullscreen(); }
		inline [[nodiscard]] bool WasMinimized() noexcept { return m_DisplayState.WasMinimized(); }
		inline [[nodiscard]] bool WasWindowed() noexcept { return m_DisplayState.WasWindowed(); }
		inline [[nodiscard]] bool WasResized() noexcept { return m_DisplayState.WasResized(); }*/

		inline [[nodiscard]] bool IsFocused() const noexcept { return m_Focused; }

		inline [[nodiscard]] const HWND Handle() const noexcept { return m_WindowHandle; }
		inline [[nodiscard]] RECT ClientArea() const noexcept { return m_ClientArea; }
	private:
		void CallCharKeyCallback(char c, bool isReleased) noexcept;

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
		CMRendererSettings& m_CMSettingsRef;
		CMWindowSettings& m_WindowSettingsRef;
		CMLoggerWide& m_CMLoggerRef;
		RECT m_ClientArea = {};
		HINSTANCE m_hInstance = nullptr;
		HWND m_WindowHandle = nullptr;
		bool m_Initialized = false;
		bool m_Shutdown = false;
		bool m_Running = false;
		bool m_Focused = false;
		bool m_Maximized = false;
		bool m_Minimized = false;
		bool m_Windowed = false;
		bool m_SetWindowedPos = false;
		//CMWindowDisplayState m_DisplayState = {};
		std::unordered_map<char, std::function<void(bool)>> m_CharKeyCallbacks;
	};
}