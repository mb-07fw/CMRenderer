#pragma once

#include "Export.hpp"
#include "Macros.hpp"
#include "Platform/Core/IGraphics.hpp"
#include "Platform/WinImpl/PlatformOSFwd_WinImpl.hpp"
#include "Platform/WinImpl/Window_WinImpl.hpp"
#include "Platform/WinImpl/ShaderLibrary_WinImpl.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"

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

	class ModelImporter
	{
	public:
		ModelImporter() = default;
		~ModelImporter() = default;

		Assimp::Importer Assimp;
	};

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
		virtual void Draw(const void* pBuffer, const DrawDescriptor& descriptor) noexcept override;

		virtual void StartFrame(const Color4& clearColor) noexcept override;
		virtual void EndFrame() noexcept override;

		virtual [[nodiscard]] BufferPtr<IUploadable> CreateConstantBuffer(GPUBufferFlag flags = GPUBufferFlag::Default, uint32_t registerSlot = 0) noexcept override;
		virtual void SetConstantBuffer(const BufferPtr<IUploadable>& pCB, void* pData, size_t numBytes) noexcept override;
		virtual [[nodiscard]] void BindConstantBufferVS(const BufferPtr<IUploadable>& pCB) noexcept override;
		virtual [[nodiscard]] void BindConstantBufferPS(const BufferPtr<IUploadable>& pCB) noexcept override;

		inline virtual [[nodiscard]] bool IsWithinFrame() const noexcept override { return m_IsWithinFrame; }
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
		ShaderLibrary m_ShaderLibrary;
		ModelImporter m_ModelImporter;
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
		bool m_IsWithinFrame = false;
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