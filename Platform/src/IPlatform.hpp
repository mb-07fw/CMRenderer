#pragma once

#include "Export.hpp"
#include "ILogger.hpp"

namespace Platform
{
    struct PlatformSettings
    {
        bool IsGraphicsDebugging = false;
    };

    /* WARNING: Lifetime of the logger is NOT managed by the platform backend. */
    [[nodiscard]] ILogger* GetActiveLogger() noexcept;
    void SetActiveLogger(ILogger* pActiveLogger) noexcept;
    void ResetActiveLogger() noexcept;

    void LogInfo(const char* pMsg) noexcept;
    void LogWarning(const char* pMsg) noexcept;
    void LogFatal(const char* pMsg) noexcept;

    class PLATFORM_API IPlatform
    {
    public:
        IPlatform() = default;
        virtual ~IPlatform() = default;

        IPlatform(const IPlatform&) = delete;
        IPlatform(IPlatform&&) = delete;
        IPlatform& operator=(const IPlatform&) = delete;
        IPlatform& operator=(IPlatform&&) = delete;
    };
}