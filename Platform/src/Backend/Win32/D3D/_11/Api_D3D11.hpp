#pragma once

#include "Backend/Win32/D3D/_11/ApiFactory_D3D11.hpp"
#include "Backend/Win32/Types_Win32.hpp"
#include "IApi.hpp"
#include "IPlatform.hpp"

#include <cstdint>

#include <filesystem>
#include <functional>
#include <type_traits>

#include <d3dcommon.h>

namespace Platform::Backend::Win32::D3D::_11
{
    class Api final : public IApi
    {
    public:
        Api(const PlatformSettings& settings, ::HWND hWnd) noexcept;
        ~Api() noexcept;
    public:
        virtual [[nodiscard]] IApiFactory& Factory() noexcept override { return m_Factory; }

        virtual void Clear(const float(&rgba)[4]) noexcept override;

        virtual void Present() noexcept override;

        virtual void Draw(
            uint32_t numVertices,
            uint32_t startVertexLocation
        ) noexcept override;

        virtual void DrawIndexed(
            uint32_t numIndices,
            uint32_t startIndexLocation,
            int32_t baseVertexLocation
        ) noexcept override;

        virtual void DrawIndexedInstanced(
            uint32_t indicesPerInstance,
            uint32_t totalInstances,
            uint32_t startIndexLocation,
            int32_t baseVertexLocation,
            uint32_t startInstanceLocation
        ) noexcept override;

        virtual void CompileShader(
            ShaderType type,
            const char* pRawShaderData,
            size_t shaderDataSizeBytes,
            IBlob& outBytecode
        ) noexcept override;

        virtual void CompileShaderAs(
            const char* pRawShaderData,
            size_t shaderDataSizeBytes,
            IVertexShader& outShader
        ) noexcept override;

        virtual void CompileShaderAs(
            const char* pRawShaderData,
            size_t shaderDataSizeBytes,
            IPixelShader& outShader
        ) noexcept override;

        virtual void InitializeInputLayout(
            IInputLayout& outInputLayout,
            const IVertexShader& vertexShader,
            const InputElement* pElems,
            size_t numElems
        ) noexcept override;

        virtual void InitializeShader(IVertexShader& outShader) noexcept override;
        virtual void InitializeShader(IPixelShader& outShader) noexcept override;

        virtual void SetBuffer(
            IVertexBuffer& vBuffer,
            const Byte* pData,
            size_t numBytes,
            uint32_t strideBytes,
            uint32_t offsetBytes,
            uint32_t registerSlot
        ) noexcept override;

        virtual void BindBuffer(IVertexBuffer& vBuffer) noexcept override;

        virtual void BindInputLayout(IInputLayout& inputLayout) noexcept override;

        virtual void BindShader(IVertexShader& vShader) noexcept override;
        virtual void BindShader(IPixelShader& pxShader) noexcept override;

        virtual [[nodiscard]] bool HasResized() noexcept override;
    private:
        void Init(const PlatformSettings& settings, ::HWND hWnd) noexcept;
        void Shutdown() noexcept;

        void CreateSwapChainViews() noexcept;
        void BindSwapChainViews() noexcept;
        void ReleaseSwapChainViews() noexcept;
        
        void SetViewport(float width, float height) noexcept;
        void SetTopology() noexcept;

        void OnWindowResize(uint32_t width, uint32_t height) noexcept override;
    
        void CompileShaderOfType(
            ShaderType type,
            const char* pRawShaderData,
            size_t shaderDataSizeBytes,
            ComPtr<::ID3DBlob>& outBytecode
        ) noexcept;

        template <typename Derived, typename Interface>
            requires (std::is_base_of_v<Interface, Derived>&&
        std::is_base_of_v<IShader, Derived>)
        inline void CompileShaderAsImpl(
            const char* pFilePath,
            size_t shaderDataSizeBytes,
            Interface& outShader
        ) noexcept;

        template <typename Derived, typename Interface>
            requires (std::is_base_of_v<Interface, Derived> &&
                std::is_base_of_v<IShader, Derived>)
        inline void InitializeShaderImpl(Interface& outShader) noexcept;

        void RetrieveDebugLayer() noexcept;
    private:
        static constexpr ::UINT S_PresentSyncIntervalTearing = 0;
        static constexpr ::UINT S_PresentSyncIntervalVSync = 1;
        ApiFactory m_Factory;
        ComPtr<::ID3D11Device> mP_Device;
		ComPtr<::ID3D11DeviceContext> mP_Context;
		ComPtr<::IDXGISwapChain> mP_SwapChain;
        ComPtr<::ID3D11RenderTargetView> mP_RTV;
		ComPtr<::ID3D11DepthStencilView> mP_DSV;
        ComPtr<::IDXGIDebug> mP_DebugInterface;
        ComPtr<::IDXGIInfoQueue> mP_InfoQueue;
        ::UINT m_PresentSyncInterval = S_PresentSyncIntervalVSync;
        ::UINT m_PresentFlags = 0;
        bool m_IsDebugLayerPresent = false;
        bool m_HasResized = false;
    };

    template <typename Derived, typename Interface>
        requires (std::is_base_of_v<Interface, Derived>&&
            std::is_base_of_v<IShader, Derived>)
    inline void Api::CompileShaderAsImpl(
        const char* pRawShaderData,
        size_t shaderDataSizeBytes,
        Interface& outShader
    ) noexcept
    {
        ShaderType type = outShader.Type();

        {
            uint16_t typeUint16 = Cast<uint16_t>(type);

            PLATFORM_FAILURE_IF_V(
                !TryCast<Derived*>(&outShader),
                "(Api_D3D11) Provided shader object isn't an instance of this api's derived "
                "shader type. ({})",
                typeUint16
            );
        }

        auto& shader = *Cast<Derived*>(&outShader);
        ComPtr<::ID3DBlob>& pBytecode = shader.Impl_GetBytecode();

        CompileShaderOfType(
            type,
            pRawShaderData,
            shaderDataSizeBytes,
            pBytecode
        );
    }

    template <typename Derived, typename Interface>
        requires (std::is_base_of_v<Interface, Derived> &&
            std::is_base_of_v<IShader, Derived>)
    inline void Api::InitializeShaderImpl(Interface& outShader) noexcept
    {
        ShaderType type = outShader.Type();

        {
            uint16_t typeUint16 = Cast<uint16_t>(type);

            PLATFORM_FAILURE_IF_V(
                !TryCast<Derived*>(&outShader),
                "(Api_D3D11) Provided shader object isn't an instance of this api's derived "
                "shader type. ({})",
                typeUint16
            );
        }

        auto& shader = *Cast<Derived*>(&outShader);
        shader.Create(mP_Device);
    }
}