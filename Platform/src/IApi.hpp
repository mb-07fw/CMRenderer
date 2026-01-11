#pragma once

#include "Export.hpp"

#include <cstdint>

namespace Platform::Graphics
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

    enum class PLATFORM_API ShaderType : int8_t
    {
        Unknown = -1,
        Vertex = 1,
        Pixel
    };

    class PLATFORM_API IApi
    {
    public:
        IApi() = default;
        virtual ~IApi() = default;

        IApi(const IApi&) = delete;
        IApi(IApi&&) = delete;
        IApi& operator=(const IApi&) = delete;
        IApi& operator=(IApi&&) = delete;

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

        virtual void CompileShader(ShaderType type, const char* pFilePath) noexcept = 0;
    };
}