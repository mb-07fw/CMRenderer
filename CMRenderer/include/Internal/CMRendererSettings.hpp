#pragma once

#include <string_view>
#include <cstdint>

namespace CMRenderer
{
	struct CMWindowData
	{
		CMWindowData() noexcept;

		CMWindowData(
			std::wstring_view windowTitle,
			bool useFullscreen,
			uint32_t width,
			uint32_t height
		) noexcept;

		std::wstring_view WindowTitle;
		unsigned int Width = 0, Height = 0;
		bool UseFullscreen = false;

		static constexpr std::wstring_view S_DEFAULT_WINDOW_TITLE = L"CMRenderer Window";
		static constexpr unsigned int S_DEFAULT_WIDTH = 800;
		static constexpr unsigned int S_DEFAULT_HEIGHT = 600;
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
		uint32_t MaxWidth, MaxHeight; // Represents the total width of the screen.
	};

	struct CMRendererSettings
	{
		CMRendererSettings(CMWindowData targetWindowSettings) noexcept;
		~CMRendererSettings() = default;

		CMWindowSettings WindowSettings;
		const bool M_FAIL_FAST = true; // TODO: Make this optional.
	};
}