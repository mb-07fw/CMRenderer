#pragma once

#define CM_NO_OP (void)0

#if defined(CM_DEBUG)

#define CM_IF_DEBUG(x) x
#define CM_IF_NDEBUG_REPLACE(x) x

#else

#define CM_IF_DEBUG(x) CM_NO_OP
#define CM_IF_NDEBUG_REPLACE(x)

#endif



#if defined(CM_RELEASE)

#define CM_IF_RELEASE(x) x

#else

#define CM_IF_RELEASE(x) CM_NO_OP

#endif



#if defined(CM_DISTRIBUTION)

#define CM_DISTRIBUTION(x) x

#else

#define CM_DISTRIBUTION(x) CM_NO_OP

#endif