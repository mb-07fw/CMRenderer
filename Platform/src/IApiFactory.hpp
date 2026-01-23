#pragma once

#include "Export.hpp"
#include "IApiResources.hpp"

namespace Platform
{
    /* Note: Any returned pointers are owning (the client manages lifetime),
     *         and must be destroyed via IRefCounted::Release, in which the
     *         underlying instance of the resource is invalidated.
     *       Any usage of a pointer after calling Release is
     *         expected to be a use-after-free. */
    class PLATFORM_API IApiFactory
    {
    public:
        IApiFactory() = default;
        virtual ~IApiFactory() = default;

        IApiFactory(const IApiFactory&) = delete;
        IApiFactory(IApiFactory&&) = delete;
        IApiFactory& operator=(const IApiFactory&) = delete;
        IApiFactory& operator=(IApiFactory&&) = delete;

        virtual [[nodiscard]] IInputLayout* CreateInputLayout() noexcept = 0;
        virtual [[nodiscard]] IVertexShader* CreateVertexShader() noexcept = 0;
        virtual [[nodiscard]] IPixelShader* CreatePixelShader() noexcept = 0;

        virtual [[nodiscard]] IBuffer* CreateBuffer(
            BufferType type,
            BufferPolicy policy
        ) noexcept = 0;

        virtual [[nodiscard]] IVertexBuffer* CreateVertexBuffer(
            BufferPolicy policy = BufferPolicy::Default
        ) noexcept = 0;

        virtual [[nodiscard]] IIndexBuffer* CreateIndexBuffer(
            BufferPolicy policy = BufferPolicy::Default
        ) noexcept = 0;

        virtual [[nodiscard]] IConstantBuffer* CreateConstantBuffer(
            BufferPolicy policy = BufferPolicy::Default
        ) noexcept = 0;

        virtual [[nodiscard]] IBlob* CreateBlob() noexcept = 0;
    };
}