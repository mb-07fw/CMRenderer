#include "Platform/Core/IPlatform.hpp"

#include <spdlog/spdlog.h>

namespace CMEngine
{
	void IPlatform::SetInterfaces(IWindow& window, IGraphics& graphics) noexcept
	{
		mP_Window = &window;
		mP_Graphics = &graphics;
	}
}