#pragma once

#ifdef CM_PLATFORM_IMPL_BUILD
	#define CM_PLATFORM_IMPL_API __declspec(dllexport)
#else
	#define CM_PLATFORM_IMPL_API __declspec(dllimport)
#endif