#pragma once

#include <iostream>

#define CM_ENGINE_WIDEN_STR2(x) L ## x
#define CM_ENGINE_WIDEN_STR(x) CM_WIDEN_STR2(x)


#ifdef _WIN32
    #define CM_ENGINE_AS_NATIVE_STR(x) CM_WIDEN_STR(x)
#else
    #define CM_ENGINE_AS_NATIVE_STR(x) x
#endif

#define CM_ENGINE_NO_OP (void)0

#if defined(CM_DEBUG) && (_MSC_VER)
    #define CM_ENGINE_BREAK_DEBUGGER()		__debugbreak()
#else
    #define CM_ENGINE_BREAK_DEBUGGER()		CM_ENGINE_NO_OP
#endif

#if defined(CM_DEBUG) || defined(_DEBUG)

#define CM_ENGINE_IF_DEBUG(x)				do { x; } while (false)
#define CM_ENGINE_IF_NDEBUG_REPLACE(x)		x
#define CM_ENGINE_ASSERT(x) \
    if (!(x)) { \
        std::cerr << "CM_ASSERT failed. (" << #x << ") | Line : " << __LINE__ << " | File : " << __FILE__ << '\n'; \
        CM_BREAK_DEBUGGER(); \
        exit(-1); \
    } \

#else

#define CM_ENGINE_IF_DEBUG(x)			CM_ENGINE_NO_OP
#define CM_ENGINE_IF_NDEBUG_REPLACE(x)
#define CM_ENGINE_ASSERT(x)		        CM_ENGINE_NO_OP

#endif

#if defined(CM_RELEASE)
    #define CM_ENGINE_IF_RELEASE(x)	x
#else
    #define CM_ENGINE_IF_RELEASE(x) CM_ENGINE_NO_OP
#endif