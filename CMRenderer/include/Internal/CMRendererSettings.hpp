#pragma once

#include <string_view>
#include <cstdint>

#include <winsdkver.h>
#include <Windows.h>

#include <windef.h>

namespace CMRenderer
{
	struct CMWindowData
	{
		CMWindowData() noexcept;

		CMWindowData(
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
		CMWindowSettings(CMWindowData targetWindowSettings) noexcept;
		~CMWindowSettings() = default;

		void SetTitleToDefault() noexcept;
		void SetWidthToDefault() noexcept;
		void SetHeightToDefault() noexcept;
		void SetFullscreenToDefault() noexcept;

		const CMWindowData DEFAULT;
		CMWindowData Current;
		LONG MaxWidth, MaxHeight; // Represents the total width of the screen.
	};

	struct CMRendererSettings
	{
		CMRendererSettings(CMWindowData targetWindowSettings) noexcept;
		~CMRendererSettings() = default;

		CMWindowSettings WindowSettings;
	};
}