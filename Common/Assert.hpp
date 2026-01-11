#pragma once

#include <iostream>
#include <exception> // std::terminate

#include "Config.hpp"

#if defined(CONFIG_REL_DEBUG) || defined(CONFIG_DEBUG)
    #define ASSERT(x, msg) if (!(x)) \
                            { \
                                std::cout << "Assertion failed: " << #x << "\n\t| Message: " << msg << '\n'; \
                                std::terminate(); \
                            }
#else
    #define ASSERT(x, msg) NO_OP
#endif