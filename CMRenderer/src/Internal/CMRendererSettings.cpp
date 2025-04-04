#include "Core/CMPCH.hpp"
#include "Internal/CMRendererSettings.hpp"

namespace CMRenderer
{
	CMWindowData::CMWindowData() noexcept
		: WindowTitle(S_DEFAULT_WINDOW_TITLE), InitialWidth(S_DEFAULT_WIDTH),
		  InitialHeight(S_DEFAULT_HEIGHT), UseFullscreen(false)
	{
	}

	CMWindowData::CMWindowData(
		std::wstring_view windowTitle,
		bool useFullscreen,
		LONG width,
		LONG height
	) noexcept
		: WindowTitle(windowTitle), UseFullscreen(useFullscreen),
		  InitialWidth(width), InitialHeight(height)
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
		Current.InitialWidth = DEFAULT.InitialWidth;
	}

	void CMWindowSettings::SetHeightToDefault() noexcept
	{
		Current.InitialHeight = DEFAULT.InitialHeight;
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