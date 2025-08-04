#pragma once

#include <minwindef.h>

#include <string_view>
#include <cstdint>

namespace CMEngine::Win
{
	struct WindowResolution
	{
		LONG Width = 0;
		LONG Height = 0;
	};

	struct WindowData
	{
		inline constexpr WindowData() noexcept;

		inline constexpr WindowData(
			std::wstring_view windowTitle,
			bool useFullscreen,
			LONG width,
			LONG height
		) noexcept;

		std::wstring_view WindowTitle;
		LONG InitialWidth = 0;
		LONG InitialHeight = 0;
		RECT ClientArea = {};
		bool UseFullscreen = false;

		static constexpr std::wstring_view S_DEFAULT_WINDOW_TITLE = L"CMEngine";
		static constexpr LONG S_DEFAULT_WIDTH = 800;
		static constexpr LONG S_DEFAULT_HEIGHT = 600;
	};

	struct WindowSettings
	{
		inline WindowSettings(WindowData targetWindowSettings) noexcept;
		~WindowSettings() = default;

		inline void SetTitleToDefault() noexcept;
		inline void SetWidthToDefault() noexcept;
		inline void SetHeightToDefault() noexcept;
		inline void SetFullscreenToDefault() noexcept;

		static constexpr WindowData S_DEFAULT;
		WindowData Current;
		LONG MaxWidth, MaxHeight; // Represents the total width of the screen.
	};

	inline constexpr WindowData::WindowData() noexcept
		: WindowTitle(S_DEFAULT_WINDOW_TITLE),
		  InitialWidth(S_DEFAULT_WIDTH),
		  InitialHeight(S_DEFAULT_HEIGHT),
		  UseFullscreen(false)
	{
	}

	inline constexpr WindowData::WindowData(
		std::wstring_view windowTitle,
		bool useFullscreen,
		LONG width,
		LONG height
	) noexcept
		: WindowTitle(windowTitle), UseFullscreen(useFullscreen),
		  InitialWidth(width), InitialHeight(height)
	{
	}

	inline WindowSettings::WindowSettings(WindowData targetWindowSettings) noexcept
		: Current(targetWindowSettings),
		  MaxWidth(GetSystemMetrics(SM_CXSCREEN)),
		  MaxHeight(GetSystemMetrics(SM_CYSCREEN))
	{
	}

	inline void WindowSettings::SetTitleToDefault() noexcept
	{
		Current.WindowTitle = S_DEFAULT.WindowTitle;
	}

	inline void WindowSettings::SetWidthToDefault() noexcept
	{
		Current.InitialWidth = S_DEFAULT.InitialWidth;
	}

	inline void WindowSettings::SetHeightToDefault() noexcept
	{
		Current.InitialHeight = S_DEFAULT.InitialHeight;
	}

	inline void WindowSettings::SetFullscreenToDefault() noexcept
	{
		Current.UseFullscreen = S_DEFAULT.UseFullscreen;
	}
}