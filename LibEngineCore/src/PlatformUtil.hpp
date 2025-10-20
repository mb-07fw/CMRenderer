#pragma once

#ifdef ENGINE_CORE_PLATFORM_WINIMPL
	#include "Platform/WinImpl/PlatformUtil_WinImpl.hpp"
#else
	#error Failed to include proper PlatformUtil implementation.
#endif

namespace CMEngine
{
#ifdef ENGINE_CORE_PLATFORM_WINIMPL
	using APlatformUtil = Platform::WinImpl::PlatformUtil;
#endif
}