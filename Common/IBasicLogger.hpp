#pragma once

class IBasicLogger
{
public:
    IBasicLogger() = default;
    virtual ~IBasicLogger() = default;

    virtual void LogInfo(const char* pMsg) noexcept = 0;
    virtual void LogWarning(const char* pMsg) noexcept = 0;
    virtual void LogFatal(const char* pMsg) noexcept = 0;

    virtual void Flush() noexcept = 0;
};