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
#include "Internal/DirectX/DXCamera.hpp"
#include "Internal/CMShapes.hpp"
#include "Internal/CMTypes.hpp"

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



	class DXContextState
	{
	public:
		DXContextState(Utility::CMLoggerWide& cmLoggerRef, DXShaderLibrary& shaderLibraryRef, Components::DXDevice& deviceRef) noexcept;
		~DXContextState() = default;
	public:
		void SetCurrentShader(DXImplementedShaderType shaderType) noexcept;

		[[nodiscard]] DXImplementedShaderType CurrentShaderType() const noexcept;
	private:
		void EnforceInitializedState() noexcept;
	public:
		DXShaderLibrary& m_ShaderLibraryRef;
		Components::DXDevice& m_DeviceRef;
		CMViewPtr<DXShaderSet> mP_CurrentShaderSet;
		Utility::CMLoggerWide& m_CMLoggerRef;
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
		void Present() noexcept;

		void DrawRect(CMRect rect) noexcept;

		//void TestDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY, float offsetZ) noexcept;
		//void TestTextureDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY, float offsetZ) noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
	private:
		void InitImGui(const HWND hWnd) noexcept;
		void ShutdownImGui() noexcept;

		void CreateRTV() noexcept;
		void BindRTV() noexcept;

		void SetViewport() noexcept;
		void SetTopology() noexcept;

		void ReleaseViews() noexcept;
	private:
		CMWindowData& m_CurrentWindowDataRef;
		DXShaderLibrary m_ShaderLibrary;
		DXContextState m_State;
		Components::DXDevice m_Device;
		Components::DXFactory m_Factory;
		Components::DXSwapChain m_SwapChain;
		CM_IF_NDEBUG_REPLACE(
			HMODULE m_DebugInterfaceModule = nullptr;
			Microsoft::WRL::ComPtr<IDXGIDebug> mP_DebugInterface;
			Components::DXInfoQueue m_InfoQueue;
		);
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> mP_RasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mP_RTV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mP_DSV;
		Utility::CMLoggerWide& m_CMLoggerRef;
		bool m_Initialized = false;
		bool m_Shutdown = false;

		/*float posX = 0.0f, posY = 0.0f, posZ = -10.0f;
		float faceOneX = 0.0f, faceOneY = 0.0f, faceOneZ = -1.0f;
		float faceTwoX = 0.0f, faceTwoY = 0.0f, faceTwoZ = 1.0f;
		float faceOneRotX = 0.0f, faceOneRotY = 0.0f;
		float faceTwoRotX = 0.0f, faceTwoRotY = 0.0f;*/
	};
}