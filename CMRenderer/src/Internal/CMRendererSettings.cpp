#include "Core/CMPCH.hpp"
#include "Internal/CMRendererSettings.hpp"

namespace CMRenderer
{
	CMWindowData::CMWindowData() noexcept
		: WindowTitle(S_DEFAULT_WINDOW_TITLE), Width(S_DEFAULT_WIDTH),
		  Height(S_DEFAULT_HEIGHT), UseFullscreen(false)
	{
	}

	CMWindowData::CMWindowData(
		std::wstring_view windowTitle,
		bool useFullscreen,
		uint32_t width,
		uint32_t height
	) noexcept
		: WindowTitle(windowTitle), UseFullscreen(useFullscreen),
		Width(width), Height(height)
	{
	}

	CMWindowSettings::CMWindowSettings(CMWindowData targetWindowSettings) noexcept
		: Current(targetWindowSettings), MaxWidth(GetSystemMetrics(SM_CXSCREEN)),
		  MaxHeight(GetSystemMetrics(SM_CYSCREEN))
	{
	}

	void CMWindowSettings::SetTitleToDefault() noexcept
	{
		Current.WindowTitle = DEFAULT.WindowTitle;
	}

	void CMWindowSettings::SetWidthToDefault() noexcept
	{
		Current.Width = DEFAULT.Width;
	}

	void CMWindowSettings::SetHeightToDefault() noexcept
	{
		Current.Height = DEFAULT.Height;
	}

	void CMWindowSettings::SetFullscreenToDefault() noexcept
	{
		Current.UseFullscreen = DEFAULT.UseFullscreen;
	}

	CMRendererSettings::CMRendererSettings(CMWindowData targetWindowSettings) noexcept
		: WindowSettings(targetWindowSettings)
	{
	}
}