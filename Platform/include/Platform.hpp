#pragma once

#ifdef _WIN32
    #include "Backend/Win32/Platform_Win32.hpp"

    namespace Platform
    {
        using Native = Platform::Backend::Win32::Platform;
    }
#else
    #error "Unimplemented platform!!!"
#endif