#pragma once

#include "Export.hpp"
#include "ILogger.hpp"

namespace Platform
{
    class PLATFORM_API IWindow
    {
    public:
        struct ResizeCallback
        {
            void (*pCallback)(uint32_t width, uint32_t height, void* pUserData) = nullptr;
            /* This should be obvious, but the lifetime of this pointer is in
             *   no way shape or form managed by the underlying IWindow instance. */
            void* pUserData = nullptr;
        };

        IWindow() = default;
        virtual ~IWindow() = default;

        /* To force derived implementations to be non-movable/copyable... */
        IWindow(const IWindow&) = delete;
        IWindow(IWindow&&) = delete;
        IWindow& operator=(const IWindow&) = delete;
        IWindow& operator=(IWindow&&) = delete;

        virtual void SetWindowResizeCallback(
            void (*pCallback)(uint32_t width, uint32_t height, void* pUserData),
            void* pUserData = nullptr
        ) noexcept = 0;

        virtual void ClearWindowResizeCallback() noexcept = 0;

        virtual void Update() noexcept = 0;

        virtual [[nodiscard]] bool IsRunning() const noexcept = 0;      
        virtual [[nodiscard]] bool IsVisible() const noexcept = 0;      
    };
}