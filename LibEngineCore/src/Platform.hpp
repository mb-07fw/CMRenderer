#pragma once

#ifdef ENGINE_CORE_PLATFORM_WINIMPL
	#include "Platform/WinImpl/Platform_WinImpl.hpp"
#else
	#error Failed to include proper Platform implementation.
#endif

#include "Graphics.hpp"

namespace CMEngine
{
#ifdef ENGINE_CORE_PLATFORM_WINIMPL
	using APlatform = Platform::WinImpl::Platform;
	using AWindow = Platform::WinImpl::Window;
#endif
}