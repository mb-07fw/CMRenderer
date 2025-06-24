#pragma once

#include <minwindef.h>

#include <string_view>
#include <cstdint>

namespace CMEngine
{
	struct CMWindowResolution
	{
		LONG Width = 0;
		LONG Height = 0;
	};

	struct CMWindowData
	{
		inline constexpr CMWindowData() noexcept;

		inline constexpr CMWindowData(
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

		static constexpr std::wstring_view S_DEFAULT_WINDOW_TITLE = L"CMRenderer Window";
		static constexpr LONG S_DEFAULT_WIDTH = 800;
		static constexpr LONG S_DEFAULT_HEIGHT = 600;
	};

	struct CMWindowSettings
	{
		inline CMWindowSettings(CMWindowData targetWindowSettings) noexcept;
		~CMWindowSettings() = default;

		inline void SetTitleToDefault() noexcept;
		inline void SetWidthToDefault() noexcept;
		inline void SetHeightToDefault() noexcept;
		inline void SetFullscreenToDefault() noexcept;

		static constexpr CMWindowData S_DEFAULT;
		CMWindowData Current;
		LONG MaxWidth, MaxHeight; // Represents the total width of the screen.
	};

	inline constexpr CMWindowData::CMWindowData() noexcept
		: WindowTitle(S_DEFAULT_WINDOW_TITLE),
		  InitialWidth(S_DEFAULT_WIDTH),
		  InitialHeight(S_DEFAULT_HEIGHT),
		  UseFullscreen(false)
	{
	}

	inline constexpr CMWindowData::CMWindowData(
		std::wstring_view windowTitle,
		bool useFullscreen,
		LONG width,
		LONG height
	) noexcept
		: WindowTitle(windowTitle), UseFullscreen(useFullscreen),
		  InitialWidth(width), InitialHeight(height)
	{
	}

	inline CMWindowSettings::CMWindowSettings(CMWindowData targetWindowSettings) noexcept
		: Current(targetWindowSettings),
		  MaxWidth(GetSystemMetrics(SM_CXSCREEN)),
		  MaxHeight(GetSystemMetrics(SM_CYSCREEN))
	{
	}

	inline void CMWindowSettings::SetTitleToDefault() noexcept
	{
		Current.WindowTitle = S_DEFAULT.WindowTitle;
	}

	inline void CMWindowSettings::SetWidthToDefault() noexcept
	{
		Current.InitialWidth = S_DEFAULT.InitialWidth;
	}

	inline void CMWindowSettings::SetHeightToDefault() noexcept
	{
		Current.InitialHeight = S_DEFAULT.InitialHeight;
	}

	inline void CMWindowSettings::SetFullscreenToDefault() noexcept
	{
		Current.UseFullscreen = S_DEFAULT.UseFullscreen;
	}
}