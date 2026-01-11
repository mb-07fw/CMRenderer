#pragma once

#ifdef PLATFORM_BUILD
    #define PLATFORM_API __declspec(dllexport)
#else
    #define PLATFORM_API __declspec(dllimport)
#endif