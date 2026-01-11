#pragma once

#include "IApi.hpp"
#include "IPlatform.hpp"
#include "Win32/PlatformOS_Win32.hpp"

#include <filesystem>

namespace Platform::Win32::Graphics::D3D::_11
{
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    using IApi = Platform::Graphics::IApi;
    using ShaderType = Platform::Graphics::ShaderType;

    class Api final : public IApi
    {
    public:
        Api(const PlatformSettings& settings, ::HWND hWnd) noexcept;
        ~Api() noexcept;

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

        virtual void CompileShader(ShaderType type, const char* pFilePath) noexcept override;
    private:
        void Init(const PlatformSettings& settings, ::HWND hWnd) noexcept;
        void Shutdown() noexcept;

        void CreateSwapChainViews() noexcept;
        void BindSwapChainViews() noexcept;
        void ReleaseSwapChainViews() noexcept;
        
        void SetViewport(float width, float height) noexcept;

        void CompileVertexShader(const std::filesystem::path& filePath) noexcept;

        void RetrieveDebugLayer() noexcept;
    private:
        static constexpr ::UINT S_PresentSyncIntervalTearing = 0;
        static constexpr ::UINT S_PresentSyncIntervalVSync = 1;
        ::UINT m_PresentSyncInterval = S_PresentSyncIntervalVSync;
        ::UINT m_PresentFlags = 0;
        bool m_IsDebugLayerPresent = false;
        ComPtr<::ID3D11Device> mP_Device;
		ComPtr<::ID3D11DeviceContext> mP_Context;
		ComPtr<::IDXGISwapChain> mP_SwapChain;
        ComPtr<::ID3D11RenderTargetView> mP_RTV;
		ComPtr<::ID3D11DepthStencilView> mP_DSV;
        ComPtr<::IDXGIDebug> mP_DebugInterface;
        ComPtr<::IDXGIInfoQueue> mP_InfoQueue;
    };
}