#pragma once

#ifdef CM_PLATFORM_BUILD
	#define CM_PLATFORM_API __declspec(dllexport)
#else
	#define CM_PLATFORM_API __declspec(dllimport)
#endif