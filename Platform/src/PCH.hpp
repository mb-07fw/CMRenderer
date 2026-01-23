#pragma once

#ifdef _WIN32
    #include "Backend/Win32/PlatformOS_Win32.hpp"
    #include "Backend/Win32/PlatformFwd_Win32.hpp"
#else
    #error "Unsupported platform!!!"
#endif

#include <cstdint>

#include <iostream>
#include <exception> // std::terminate
#include <format>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>