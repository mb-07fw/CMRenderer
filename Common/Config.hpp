#pragma once

#define NO_OP (void)0

#ifdef CONFIG_DEBUG
    inline constexpr bool G_Config_IsDebug = true;
    #define IF_DEBUG(x) x
#else
    inline constexpr bool G_Config_IsDebug = false;
    #define IF_DEBUG(x) NO_OP
#endif

#ifdef CONFIG_RELEASE
    inline constexpr bool G_Config_IsRelease = true;
    #define IF_RELEASE(x) x
#else
    inline constexpr bool G_Config_IsRelease = false;
    #define IF_RELEASE(x) NO_OP
#endif

#ifdef CONFIG_REL_DEBUG
   inline constexpr bool G_Config_IsRelDebug = true;
    #define IF_REL_DEBUG(x) x
#else
    inline constexpr bool G_Config_IsRelDebug = false;
    #define IF_REL_DEBUG(x) NO_OP
#endif

#ifdef CONFIG_MIN_SIZE_RELEASE
    inline constexpr bool G_Config_IsMinSizeRelease = true;

    #define IF_MIN_SIZE_RELEASE(x) x
    #define IF_NMIN_SIZE_RELEASE(x) NO_OP

    #define CONFIG_NDIST NO_OP
#else
    inline constexpr bool G_Config_IsMinSizeRelease = false;

    #define IF_MIN_SIZE_RELEASE(x) NO_OP
    #define IF_NMIN_SIZE_RELEASE(x) x

    #define CONFIG_NDIST x
#endif

inline constexpr bool G_Config_IsDist = G_Config_IsMinSizeRelease;

#define IF_DIST(x) IF_MIN_SIZE_RELEASE(x)
#define IF_NDIST(x) IF_NMIN_SIZE_RELEASE(x)
