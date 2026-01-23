#pragma once

#include "IPlatform.hpp"
#include "Common/RuntimeFailure.hpp"

#define PLATFORM_FAILURE(msg)				  RUNTIME_FAILURE(msg, GetActiveLogger())
#define PLATFORM_FAILURE_V(fmt, ...)		  RUNTIME_FAILURE_V(GetActiveLogger(), fmt, __VA_ARGS__)
#define PLATFORM_FAILURE_IF(x, msg)			  RUNTIME_FAILURE_IF(x, msg, GetActiveLogger())
#define PLATFORM_FAILURE_IF_V(x, fmt, ...)	  RUNTIME_FAILURE_IF_V(x, GetActiveLogger(), fmt, __VA_ARGS__)