#pragma once

#include "Internal/CMRendererSettings.hpp"
#include "Internal/CMLogger.hpp"

#include <minwindef.h> // Include Windows type definitions.

namespace CMRenderer
{
	class CMWindow
	{
	public:
		CMWindow(CMRendererSettings& cmSettingsRef, CMLoggerWide& cmLoggerRef) noexcept;
		~CMWindow() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;

		void HandleMessages() noexcept;

		void Minimize() const noexcept;

		inline [[nodiscard]] bool IsInitialized() { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() { return m_Shutdown; }
		inline [[nodiscard]] bool IsRunning() { return m_Running; }
		inline [[nodiscard]] const HWND Handle() const { return m_WindowHandle; }
		inline [[nodiscard]] RECT ClientArea() const noexcept { return m_ClientArea; }
	private:
		void ValidateSettings() noexcept;
		void LogCurrentSettings() const noexcept;

		bool Create() noexcept;
		bool Destroy() noexcept;

		[[nodiscard]] HINSTANCE GetCurrentHINSTANCE() noexcept;
		void RegisterWindowClass(const WNDCLASSEX& wndClassRef) noexcept;

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
	};
}