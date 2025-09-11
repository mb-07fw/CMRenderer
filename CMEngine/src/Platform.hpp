#pragma once

#ifdef CM_ENGINE_PLATFORM_WINIMPL
	#include "Platform/WinImpl/Platform_WinImpl.hpp"
#else
	#error Failed to include proper Platform implementation.
#endif

namespace CMEngine
{
#ifdef CM_ENGINE_PLATFORM_WINIMPL
	using PlatformAlias = Platform::WinImpl::Platform;
	using GraphicsAlias = Platform::WinImpl::Graphics;
	using WindowAlias = Platform::WinImpl::Window;
#endif
}