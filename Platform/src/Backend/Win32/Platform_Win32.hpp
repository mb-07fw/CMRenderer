#pragma once

#include "IPlatform.hpp"
#include "IWindow.hpp"
#include "IContext.hpp"

namespace Platform::Backend::Win32
{
    class PLATFORM_API Platform final : public IPlatform
    {
    public:
        Platform() noexcept;
        ~Platform() noexcept;

        void CreateGraphicsContext(ApiType api) noexcept;

        [[nodiscard]] IWindow& Window() noexcept;
        [[nodiscard]] IContext& Context() noexcept;

        /* These are shorthand functions so I don't have to type platform.context().api()... */
        virtual IApi& GetGraphicsApi() noexcept override;
        virtual IApiFactory& GetGraphicsApiFactory() noexcept override;
    private:
        PlatformSettings m_Settings;
        struct Impl;
        Impl* mP_Impl = nullptr;
    };
}