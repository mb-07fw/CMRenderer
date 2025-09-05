#pragma once

#include "CMPlatform/IGraphics.hpp"
#include "CMPlatform/PlatformTypes.hpp"
#include "CMPlatform/Export.hpp"
#include "CMPlatform/WinImpl_Window.hpp"
#include "CMPlatform/WinImpl_ShaderLibrary.hpp"

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_5.h>
#include <wrl/client.h>

#include <array>
#include <vector>
#include <string_view>

#include <cstdint>

namespace CMEngine::Platform::WinImpl
{
	template <typename Ty>
	using ComPtr = Microsoft::WRL::ComPtr<Ty>;

	class Graphics : public IGraphics
	{
	public:
		Graphics(Window& window) noexcept;
		~Graphics() noexcept;

		Graphics(const Graphics& other) = delete;
		Graphics& operator=(const Graphics& other) = delete;
	public:
		//void Impl_NewFrame() noexcept;
		//void Impl_EndFrame() noexcept;

		void Impl_Update() noexcept;

		void Impl_Clear(RGBANorm color) noexcept;
		void Impl_Present() noexcept;
		void Impl_Draw(const void* pBuffer, const DrawDescriptor& descriptor) noexcept;
	private:
		void Impl_Init() noexcept;
		void Impl_Shutdown() noexcept;

		//void Impl_InitPipeline() noexcept;
		//void Impl_InitImGui() noexcept;

		//void Impl_ShutdownPipeline() noexcept;
		//void Impl_ShutdownImGui() noexcept;

		///* (Views in this context refer to RTV's and DSV's on the swap chain) */
		void Impl_CreateViews() noexcept;
		void Impl_BindViews() noexcept;
		void Impl_ReleaseViews() noexcept;

		void Impl_SetViewport() noexcept;

		void Impl_OnResizeCallback(Float2 res) noexcept;

		static void Impl_OnResizeThunk(Float2 res, void* pThis) noexcept;

		//void Impl_GetMessages(std::vector<std::wstring>& outMessages) noexcept;
	private:
		Window& m_Window; /* TODO: Come up with a better solution for this dependancy... */
		ShaderLibrary m_ShaderLibrary;
		ComPtr<ID3D11Device> mP_Device;
		ComPtr<ID3D11DeviceContext> mP_Context;
		ComPtr<IDXGISwapChain> mP_SwapChain;
		ComPtr<ID3D11RenderTargetView> mP_RTV;
		ComPtr<ID3D11DepthStencilView> mP_DSV;
		ComPtr<IDXGIDebug> mP_DebugInterface;
		ComPtr<IDXGIInfoQueue> mP_InfoQueue;
		static constexpr UINT S_PRESENT_SYNC_INTERVAL_TEARING = 0;
		static constexpr UINT S_PRESENT_SYNC_INTERVAL_VSYNC = 1;
		UINT m_PresentSyncInterval = S_PRESENT_SYNC_INTERVAL_VSYNC;
		UINT m_PresentFlags = 0;
	};

	CM_DYNAMIC_LOAD void WinImpl_Graphics_Clear(RGBANorm color);
	CM_DYNAMIC_LOAD void WinImpl_Graphics_Present();
	CM_DYNAMIC_LOAD void WinImpl_Graphics_Draw(const void* pBuffer, const DrawDescriptor* pDescriptor);
}