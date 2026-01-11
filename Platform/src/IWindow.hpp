#pragma once

#include "Export.hpp"
#include "ILogger.hpp"

namespace Platform
{
    class PLATFORM_API IWindow
    {
    public:
        IWindow() = default;
        virtual ~IWindow() = default;

        /* To force derived implementations to be non-movable/copyable... */
        IWindow(const IWindow&) = delete;
        IWindow(IWindow&&) = delete;
        IWindow& operator=(const IWindow&) = delete;
        IWindow& operator=(IWindow&&) = delete;

        virtual void Update() noexcept = 0;

        virtual [[nodiscard]] bool IsRunning() const noexcept = 0;       
    };
}