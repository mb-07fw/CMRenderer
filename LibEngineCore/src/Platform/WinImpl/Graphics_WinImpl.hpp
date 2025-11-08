#pragma once

#include "Macros.hpp"
#include "Platform/Core/IGraphics.hpp"
#include "Platform/WinImpl/PlatformOSFwd_WinImpl.hpp"
#include "Platform/WinImpl/Window_WinImpl.hpp"
#include "Platform/WinImpl/ShaderRegistry_WinImpl.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"

#include <assimp/Importer.hpp>

#include <d3d11.h>

#include <d2d1.h>
#include <dwrite.h>

#include <dxgi.h>
#include <dxgi1_5.h>
#include <dxgidebug.h>

#include <wrl/client.h>

#include <cstdint>
#include <array>
#include <vector>
#include <string_view>

namespace CMEngine::Platform::WinImpl
{
	struct PlatformConfig;

	class Graphics : public IGraphics
	{
	public:
		Graphics(Window& window, const PlatformConfig& platformConfig) noexcept;
		~Graphics() noexcept;

		Graphics(const Graphics& other) = delete;
		Graphics& operator=(const Graphics& other) = delete;
	public:
		virtual void Clear(const Color4& color) noexcept override;
		virtual void Present() noexcept override;

		virtual void Draw(uint32_t numVertices, uint32_t startVertexLocation) noexcept override;

		void DrawIndexed(uint32_t numIndices, uint32_t startIndexLocation, int32_t baseVertexLocation) noexcept;

		virtual void DrawIndexedInstanced(
			uint32_t indicesPerInstance,
			uint32_t totalInstances,
			uint32_t startIndexLocation,
			int32_t baseVertexLocation,
			uint32_t startInstanceLocation
		) noexcept override;

		virtual [[nodiscard]] Resource<IInputLayout> CreateInputLayout(ShaderID vertexID, std::span<const InputElement> elements) noexcept override;
		virtual void BindInputLayout(const Resource<IInputLayout>& pInputLayout) noexcept override;
		void DumpInputLayout(const Resource<IInputLayout>& pInputLayout) noexcept;

		virtual [[nodiscard]] Resource<IBuffer> CreateBuffer(GPUBufferType type, GPUBufferFlag flags = GPUBufferFlag::Default) noexcept override;
		virtual void SetBuffer(const Resource<IBuffer>& pBuffer, const void* pData, size_t numBytes) noexcept override;

		virtual void BindVertexBuffer(const Resource<IBuffer>& pBuffer, uint32_t strideBytes, uint32_t offsetBytes, uint32_t slot) noexcept override;
		virtual void BindIndexBuffer(const Resource<IBuffer>& pBuffer, DataFormat indexFormat, uint32_t startIndex) noexcept override;
		virtual void BindConstantBufferVS(const Resource<IBuffer>& pBuffer, uint32_t slot) noexcept override;
		virtual void BindConstantBufferPS(const Resource<IBuffer>& pBuffer, uint32_t slot) noexcept override;

		[[nodiscard]] ShaderID GetShader(std::wstring_view shaderName) noexcept;
		void BindShader(ShaderID id) noexcept;

		[[nodiscard]] ShaderID LastVS() const noexcept;
		[[nodiscard]] ShaderID LastPS() const noexcept;
	private:
		void Init() noexcept;
		void Shutdown() noexcept;

		/* (Views in this context refer to RTV's and DSV's on the swap chain) */
		void CreateViews() noexcept;
		void BindViews() noexcept;
		void ReleaseViews() noexcept;

		void SetViewport(Float2 resolution) noexcept;

		void OnResizeCallback(Float2 res) noexcept;
		static void OnResizeThunk(Float2 res, void* pThis) noexcept;

		[[nodiscard]] bool IsGraphicsDebugging() const noexcept;

		void InitDWrite() noexcept;
		void CreateD2DViews(const ComPtr<ID3D11Texture2D>& pBackBuffer) noexcept;
		void ReleaseD2DViews() noexcept;

		void D2DBeginDraw() noexcept;
		void D2DEndDraw() noexcept;

		void D2DDrawText(const std::wstring_view& text, const Float2& pos, const Float2& resolution, const Color4& color) noexcept;
		void D2DDrawRect(const Float2& pos, const Float2& resolution, const Color4& color) noexcept;

		inline [[nodiscard]] D2D1::ColorF ToD2D1ColorF(const Color4& color) const noexcept;
		inline [[nodiscard]] D2D1_RECT_F ToD2D1RectF(const Float2& pos, const Float2& resolution) const noexcept;
	private:
		Window& m_Window; /* TODO: Come up with a better solution for this dependency... */
		const PlatformConfig& m_Config;
		ShaderRegistry m_ShaderRegistry;
		ComPtr<ID3D11Device> mP_Device;
		ComPtr<ID3D11DeviceContext> mP_Context;
		ComPtr<IDXGISwapChain> mP_SwapChain;
		ComPtr<ID3D11RenderTargetView> mP_RTV;
		ComPtr<ID3D11DepthStencilView> mP_DSV;
		ComPtr<IDXGIDebug> mP_DebugInterface;
		ComPtr<IDXGIInfoQueue> mP_InfoQueue;
		ComPtr<ID2D1Factory> mP_D2D_Factory;
		ComPtr<IDWriteFactory> mP_DW_Factory;
		ComPtr<IDWriteTextFormat> mP_DW_TextFormat;
		ComPtr<ID2D1RenderTarget> mP_D2D_RT;
		ComPtr<ID2D1SolidColorBrush> mP_D2D_SC_Brush;
		static constexpr UINT S_PRESENT_SYNC_INTERVAL_TEARING = 0;
		static constexpr UINT S_PRESENT_SYNC_INTERVAL_VSYNC = 1;
		UINT m_PresentSyncInterval = S_PRESENT_SYNC_INTERVAL_VSYNC;
		UINT m_PresentFlags = 0;
		bool m_LoadedDebugLayer = false;
	};

	inline [[nodiscard]] D2D1::ColorF Graphics::ToD2D1ColorF(const Color4& color) const noexcept
	{
		return D2D1::ColorF(
			color.r(),
			color.g(),
			color.b(),
			color.a()
		);
	}

	inline [[nodiscard]] D2D1_RECT_F Graphics::ToD2D1RectF(const Float2& pos, const Float2& resolution) const noexcept
	{
		return D2D1::RectF(
			pos.x,
			pos.y,
			resolution.x + pos.x,
			resolution.y + pos.y
		);
	}
}