#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <minwindef.h>
#include <DirectXMath.h>

#include <string_view>

#include "Core/CMMacros.hpp"
#include "Internal/Utility/CMLogger.hpp"
#include "Internal/DirectX/DXComponents.hpp"
#include "Internal/DirectX/DXShaderLibrary.hpp"

namespace CMRenderer::CMDirectX
{
	struct NormColor {
		float rgba[4];
	};

	struct CBViewportSize
	{
		long Width;
		long Height;
		long padding[2] = { 0, 0 };
	};

	class DXContext
	{
	public:
		DXContext(Utility::CMLoggerWide& cmLoggerRef, CMWindowData& currentWindowData) noexcept;
		~DXContext() noexcept;
	public:
		void Init(const HWND hWnd) noexcept;
		void Shutdown() noexcept;

		void Clear(NormColor normColor) noexcept;
		void TestDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY) noexcept;
		void TestTextureDraw() noexcept;
		void Present() noexcept;

		//void Draw(CMRect rect) noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
	private:
		void CreateRTV() noexcept;
		void SetViewport() noexcept;
		void SetTopology() noexcept;

		void ReleaseViews() noexcept;
	private:
		Utility::CMLoggerWide& m_CMLoggerRef;
		CMWindowData& m_CurrentWindowDataRef;
		const HWND m_WindowHandle = nullptr;
		Components::DXDevice m_Device;
		Components::DXFactory m_Factory;
		Components::DXSwapChain m_SwapChain;
		CM_IF_NDEBUG_REPLACE(
			HMODULE m_DebugInterfaceModule = nullptr;
			Microsoft::WRL::ComPtr<IDXGIDebug> mP_DebugInterface;
			Components::DXInfoQueue m_InfoQueue;
		)
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mP_RTV;
		CMShaderLibrary m_ShaderLibrary;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}