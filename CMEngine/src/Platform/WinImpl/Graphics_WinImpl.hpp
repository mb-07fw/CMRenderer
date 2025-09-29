#pragma once

#include "Export.hpp"
#include "Macros.hpp"
#include "Platform/Core/IGraphics.hpp"
#include "Platform/WinImpl/PlatformOSFwd_WinImpl.hpp"
#include "Platform/WinImpl/Window_WinImpl.hpp"
#include "Platform/WinImpl/ShaderLibrary_WinImpl.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"

#include <dxgi.h>
#include <dxgi1_5.h>
#include <dxgidebug.h>

#include <d3d11.h>

#include <d2d1.h>
#include <dwrite.h>

#include <wrl/client.h>

#include <cstdint>
#include <array>
#include <vector>
#include <string_view>

namespace CMEngine::Platform::WinImpl
{
	class ModelImporterImpl;
	class PlatformConfig;

	class CM_ENGINE_API Graphics : public IGraphics
	{
	public:
		Graphics(Window& window, const PlatformConfig& platformConfig) noexcept;
		~Graphics() noexcept;

		Graphics(const Graphics& other) = delete;
		Graphics& operator=(const Graphics& other) = delete;
	public:
		void Update() noexcept;

		virtual void Clear(RGBANorm color) noexcept override;
		virtual void Present() noexcept override;
		virtual void Draw(const void* pBuffer, const DrawDescriptor& descriptor) noexcept override;
	private:
		void Init() noexcept;
		void Shutdown() noexcept;

		/* (Views in this context refer to RTV's and DSV's on the swap chain) */
		void CreateViews() noexcept;

		void BindViews() noexcept;

		void ReleaseViews() noexcept;

		void SetViewport() noexcept;

		void OnResizeCallback(Float2 res) noexcept;

		static void OnResizeThunk(Float2 res, void* pThis) noexcept;

		[[nodiscard]] bool IsGraphicsDebugging() const noexcept;

		void InitDWrite() noexcept;
		void CreateD2DViews(const ComPtr<ID3D11Texture2D>& pBackBuffer) noexcept;
		void ReleaseD2DViews() noexcept;

		void D2DBeginDraw() noexcept;
		void D2DEndDraw() noexcept;

		void D2DDrawText(const std::wstring_view& text, const Float2& pos, const Float2& resolution, const RGBANorm& color) noexcept;
		void D2DDrawRect(const Float2& pos, const Float2& resolution, const RGBANorm& color) noexcept;

		inline [[nodiscard]] D2D1::ColorF ToD2D1ColorF(const RGBANorm& color) const noexcept;
		inline [[nodiscard]] D2D1_RECT_F ToD2D1RectF(const Float2& pos, const Float2& resolution) const noexcept;
	private:
		Window& m_Window; /* TODO: Come up with a better solution for this dependency... */
		const PlatformConfig& m_Config;
		ShaderLibrary m_ShaderLibrary;
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
		Float3 m_CameraOffset = Float3(0.0f, 0.0f, -10.0f);
		RigidTransform m_CameraTransform = RigidTransform(Float3{}, Float3{ 0.0f, 0.0f, -10.0f });
		Float3 m_MeshOffset;
		Float2 m_TextOffset;
		Float2 m_TextResolution = Float2(250.0f, 200.0f);
		RGBANorm m_TextBoundsRGBA = RGBANorm(0.15f, 0.15f, 0.15f);
		std::unique_ptr<ModelImporterImpl> mP_ModelImporter;
		bool m_ShowTextBounds = false;
		bool m_LoadedDebugLayer = false;
	};

	inline [[nodiscard]] D2D1::ColorF Graphics::ToD2D1ColorF(const RGBANorm& color) const noexcept
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