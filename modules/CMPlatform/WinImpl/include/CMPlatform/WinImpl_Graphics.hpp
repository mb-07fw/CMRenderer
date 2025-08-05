#pragma once

#include "CMPlatform/IGraphics.hpp"
#include "CMPlatform/PlatformDef.hpp"

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_5.h>
#include <wrl/client.h>

#include <array>
#include <vector>
#include <string_view>

namespace CMEngine::Platform::WinImpl
{
	class ShaderLibrary
	{
	public:
		ShaderLibrary() noexcept;
		~ShaderLibrary() noexcept;
	public:
	private:
		void Init() noexcept;
		void Shutdown() noexcept;
	private:
		static constexpr std::array<std::wstring_view, 1> S_STOCK_VERTEX_SHADER_NAMES = {
			std::wstring_view(L"Quad_VS.cso")
		};
		static constexpr std::array<std::wstring_view, 1> S_STOCK_PIXEL_SHADER_NAMES = {
			std::wstring_view(L"Quad_PS.cso")
		};
	};

	class Graphics : public IGraphics
	{
	public:
		Graphics(HWND hWnd) noexcept;
		~Graphics() noexcept;

		Graphics(const Graphics& other) = delete;
		Graphics& operator=(const Graphics& other) = delete;
	public:
		void Impl_Update() noexcept;
	private:
		void Impl_Init() noexcept;
		void Impl_Shutdown() noexcept;

		void Impl_BindViews() noexcept;
	private:
		HWND mP_HWND = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Device> mP_Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> mP_Context;
		Microsoft::WRL::ComPtr<IDXGIFactory5> mP_Factory;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> mP_SwapChain;
		Microsoft::WRL::ComPtr<ID3D11InfoQueue> mP_InfoQueue;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mP_RTV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mP_DSV;
		static constexpr UINT S_PRESENT_SYNC_INTERVAL_TEARING = 0;
		static constexpr UINT S_PRESENT_SYNC_INTERVAL_VSYNC = 1;
		UINT m_PresentSyncInterval = S_PRESENT_SYNC_INTERVAL_VSYNC;
		UINT m_PresentFlags = 0;
	};
}