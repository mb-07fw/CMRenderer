#pragma once

#ifdef CM_ENGINE_PLATFORM_WINIMPL
	#include "Platform/WinImpl/PlatformUtil_WinImpl.hpp"
#else
	#error Failed to include proper PlatformUtil implementation.
#endif

namespace CMEngine
{
	using APlatformUtil = Platform::WinImpl::PlatformUtil;
}