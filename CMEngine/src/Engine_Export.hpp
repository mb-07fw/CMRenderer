#pragma once

#ifdef CM_ENGINE_BUILD
	#define CM_ENGINE_API __declspec(dllexport)
#else
	#define CM_ENGINE_API __declspec(dllimport)
	#define CM_ENGINE_USER
#endif