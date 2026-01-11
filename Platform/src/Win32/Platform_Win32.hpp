#pragma once

#include "IPlatform.hpp"
#include "IWindow.hpp"
#include "IContext.hpp"

namespace Platform::Win32
{
    class PLATFORM_API Platform final : public IPlatform
    {
    public:
        Platform() noexcept;
        ~Platform() noexcept;

        void CreateGraphicsContext(Graphics::ApiType api) noexcept;

        [[nodiscard]] IWindow& Window() noexcept;
        [[nodiscard]] Graphics::IContext& Context() noexcept;
    private:
        PlatformSettings m_Settings;
        struct Impl;
        Impl* mP_Impl = nullptr;
    };
}