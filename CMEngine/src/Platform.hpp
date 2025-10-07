#pragma once

#ifdef CM_ENGINE_PLATFORM_WINIMPL
	#include "Platform/WinImpl/Platform_WinImpl.hpp"
#else
	#error Failed to include proper Platform implementation.
#endif

namespace CMEngine
{
#ifdef CM_ENGINE_PLATFORM_WINIMPL
	using APlatform = Platform::WinImpl::Platform;
	using AGraphics = Platform::WinImpl::Graphics;
	using AWindow = Platform::WinImpl::Window;
#endif
}