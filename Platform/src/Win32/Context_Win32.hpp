#pragma once

#include "IContext.hpp"
#include "Win32/PlatformFwd_Win32.hpp"

#include <memory>

namespace Platform::Win32::Graphics
{
    namespace Gfx = ::Platform::Graphics;

    using IContext = Gfx::IContext;
    using IApi = Gfx::IApi;
    using ApiType = Gfx::ApiType;

    class Context final : public IContext
    {
    public:
        Context() = default;
        ~Context() = default;

        [[nodiscard]] bool Impl_Create(
            ApiType api,
            const PlatformSettings& settings,
            ::HWND hWnd
        ) noexcept;

        virtual [[nodiscard]] IApi& Api() noexcept override;
    private:
        virtual [[nodiscard]] bool Create(
            ApiType api,
            const PlatformSettings& settings,
            void* pOSWindowHandle
        ) noexcept override;

        void CreateDefaultApi(const PlatformSettings& settings, ::HWND hWnd) noexcept;
        void CreateDirect3D11Api(const PlatformSettings& settings, ::HWND hWnd) noexcept;
        void CreateDirect3D12Api() noexcept;
        void CreateOpenGLApi() noexcept;
    private:
        std::unique_ptr<IApi> mP_Api;
    };
}