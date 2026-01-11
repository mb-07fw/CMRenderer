#pragma once

#ifdef _WIN32
    #include "Win32/Platform_Win32.hpp"

    namespace Platform
    {
        using Native = Platform::Win32::Platform;
    }
#else
    #error "Unimplemented platform!!!"
#endif