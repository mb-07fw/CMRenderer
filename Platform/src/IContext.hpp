#pragma once

#include "Export.hpp"
#include "IPlatform.hpp"
#include "IApi.hpp"

namespace Platform::Graphics
{
    class PLATFORM_API IContext
    {
    public:
        IContext() = default;
        virtual ~IContext() = default;

        virtual [[nodiscard]] IApi& Api() noexcept = 0;
    protected:
        virtual [[nodiscard]] bool Create(
            ApiType impl,
            const PlatformSettings& settings,
            void* pOSWindowHandle
        ) noexcept = 0;
    };
}