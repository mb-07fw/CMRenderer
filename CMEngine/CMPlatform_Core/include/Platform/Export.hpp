#pragma once

#ifdef CM_BUILD

#define CM_API __declspec(dllexport)

#else

#define CM_API __declspec(dllimport)
#define CM_USER

#endif

#ifdef _WIN32
	#define CM_DYNAMIC_LOAD extern "C" __declspec(dllexport)
#else
	#define CM_DYNAMIC_LOAD extern "C" __attribute__((visibility("default")))
#endif