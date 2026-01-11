#pragma once

#include "Export.hpp"
#include "Common/IBasicLogger.hpp"

namespace Platform
{
    class PLATFORM_API ILogger : public IBasicLogger
    {
    public:
        ILogger() = default;
        virtual ~ILogger() = default;
    };
}