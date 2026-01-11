#include "IPlatform.hpp"

namespace Platform
{
    ILogger* gP_ActiveLogger = nullptr;

    [[nodiscard]] ILogger* GetActiveLogger() noexcept
    {
        return gP_ActiveLogger;
    }

    void SetActiveLogger(ILogger* pActiveLogger) noexcept
    {
        gP_ActiveLogger = pActiveLogger;
    }

    void ResetActiveLogger() noexcept
    {
        gP_ActiveLogger = nullptr;
    }

    void LogInfo(const char* pMsg) noexcept
    {
        if (gP_ActiveLogger)
            gP_ActiveLogger->LogInfo(pMsg);
    }

    void LogWarning(const char* pMsg) noexcept
    {
        if (gP_ActiveLogger)
            gP_ActiveLogger->LogWarning(pMsg);
    }

    void LogFatal(const char* pMsg) noexcept
    {
        if (gP_ActiveLogger)
        {
            gP_ActiveLogger->LogFatal(pMsg);
            gP_ActiveLogger->Flush();
        }
    }
}