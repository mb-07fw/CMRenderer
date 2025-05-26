#pragma once

#define CM_NO_OP (void)0

#if defined(CM_DEBUG) && (_MSC_VER)

#define CM_BREAK_DEBUGGER()		__debugbreak()

#else

#define CM_BREAK_DEBUGGER()		CM_NO_OP

#endif

#if defined(CM_DEBUG) || defined(_DEBUG)

#define CM_IF_DEBUG(x)				x
#define CM_IF_NDEBUG_REPLACE(x)		x
#define CM_ASSERT(x)				if (!(x)) \
									{ \
										std::cerr << "CM_ASSERT failed. (" << #x << ") | Line : " << __LINE__ << " | File : " << __FILE__ << '\n'; \
										CM_BREAK_DEBUGGER(); \
										exit(-1); \
									}

#else

#define CM_IF_DEBUG(x)				CM_NO_OP
#define CM_IF_NDEBUG_REPLACE(x)
#define CM_ASSERT(x)				CM_NO_OP

#endif

#if defined(CM_RELEASE)

#define CM_IF_RELEASE(x)		x

#else

#define CM_IF_RELEASE(x)		CM_NO_OP

#endif



#if defined(CM_DISTRIBUTION)

#define CM_IF_DISTRIBUTION(x)	x

#else

#define CM_IF_DISTRIBUTION(x)	CM_NO_OP

#endif