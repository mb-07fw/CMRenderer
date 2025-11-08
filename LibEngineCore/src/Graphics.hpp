#pragma once

#ifdef ENGINE_CORE_PLATFORM_WINIMPL
#include "Platform/WinImpl/Graphics_WinImpl.hpp"
#else
#error Failed to include proper Platform::Graphics implementation.
#endif

namespace CMEngine
{
#ifdef ENGINE_CORE_PLATFORM_WINIMPL
	using AGraphics = Platform::WinImpl::Graphics;
#endif
}