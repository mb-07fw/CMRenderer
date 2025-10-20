#pragma once

#ifdef ENGINE_CORE_PLATFORM_WINIMPL
	#include "Platform/WinImpl/PCH_WinImpl.hpp"
#else
	#error "Failed to include proper PCH."
#endif