#pragma once

#include "Export.hpp"
#include "IApiFactory.hpp"

#include <cstdint>

namespace Platform
{
    enum class PLATFORM_API ApiType : uint8_t;

#ifdef _WIN32
    enum class PLATFORM_API ApiType : uint8_t
    {
        Default,
        Direct3D_11,
        Direct3D_12,
        OpenGL
    };
#else
    #error "Unsupported platform!!!"
#endif

    class PLATFORM_API IApi
    {
    public:
        IApi() = default;
        virtual ~IApi() = default;

        IApi(const IApi&) = delete;
        IApi(IApi&&) = delete;
        IApi& operator=(const IApi&) = delete;
        IApi& operator=(IApi&&) = delete;
    public:
        virtual [[nodiscard]] IApiFactory& Factory() noexcept = 0;

        virtual void Clear(
            const float(&rgba)[4]
        ) noexcept = 0;

        virtual void Present() noexcept = 0;

        virtual void Draw(
            uint32_t numVertices,
            uint32_t startVertexLocation
        ) noexcept = 0;

        virtual void DrawIndexed(
            uint32_t numIndices,
            uint32_t startIndexLocation,
            int32_t baseVertexLocation
        ) noexcept = 0;

        virtual void DrawIndexedInstanced(
            uint32_t indicesPerInstance,
            uint32_t totalInstances,
            uint32_t startIndexLocation,
            int32_t baseVertexLocation,
            uint32_t startInstanceLocation
        ) noexcept = 0;

        virtual void CompileShader(
            ShaderType type,
            const char* pRawShaderData,
            size_t shaderDataSizeBytes,
            IBlob& outBytecode
        ) noexcept = 0;

        /* Assigns compiled bytecode from a shader file directly to the shader,
         * instead of requiring a separate allocated IBlob. */
        virtual void CompileShaderAs(
            const char* pRawShaderData,
            size_t shaderDataSizeBytes,
            IVertexShader& outShader 
        ) noexcept = 0;

        virtual void CompileShaderAs(
            const char* pRawShaderData,
            size_t shaderDataSizeBytes,
            IPixelShader& outShader
        ) noexcept = 0;

        virtual void InitializeInputLayout(
            IInputLayout& outInputLayout,
            const IVertexShader& vertexShader,
            const InputElement* pElems,
            size_t numElems
        ) noexcept = 0;
        
        /* It's assumed the shader object already has valid bytecode.
         *   (ex. via CompileShaderAs) */
        virtual void InitializeShader(IVertexShader& outShader) noexcept = 0;
        virtual void InitializeShader(IPixelShader& outShader) noexcept = 0;

        virtual void SetBuffer(
            IVertexBuffer& vBuffer,
            const Byte* pData,
            size_t numBytes,
            uint32_t strideBytes,
            uint32_t offsetBytes,
            uint32_t registerSlot
        ) noexcept = 0;

        virtual void BindBuffer(IVertexBuffer& vBuffer) noexcept = 0;

        virtual void BindInputLayout(IInputLayout& inputLayout) noexcept = 0;

        virtual void BindShader(IVertexShader& vShader) noexcept = 0;
        virtual void BindShader(IPixelShader& pxShader) noexcept = 0;

        virtual [[nodiscard]] bool HasResized() noexcept = 0;
        
        /* TODO: make this not public... */
        virtual void OnWindowResize(uint32_t width, uint32_t height) noexcept = 0;
    };
}