#include "IPlatform.hpp"

#include <spdlog/spdlog.h>

namespace CMPlatform
{
	IPlatform::IPlatform(
		IWindow* pWindow,
		IGraphics* pGraphics
	) noexcept
		: mP_Window(pWindow),
		  mP_Graphics(pGraphics)
	{
	}
}