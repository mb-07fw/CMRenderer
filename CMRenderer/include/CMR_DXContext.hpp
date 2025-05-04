#pragma once

#include <d3d11.h>
#include <dxgidebug.h>

#include <wrl/client.h>
#include <DirectXMath.h>

#include <imgui/imgui.h>

#include <memory>
#include <span>

#include "CMC_Macros.hpp"
#include "CMC_Logger.hpp"
#include "CMC_Types.hpp"
#include "CMR_DXComponents.hpp"
#include "CMR_DXShaderLibrary.hpp"
#include "CMR_DXCamera.hpp"

namespace CMRenderer
{
	class CMRenderer; // Forward declare here for friend declaration...
}

namespace CMRenderer::CMDirectX
{
	struct CBViewportSize
	{
		long Width;
		long Height;
		long padding[2] = { 0, 0 };
	};

	class DXContextState
	{
	public:
		DXContextState(
			CMCommon::CMLoggerWide& cmLoggerRef,
			DXShaderLibrary& shaderLibraryRef,
			Components::DXDevice& deviceRef,
			const CMWindowData& currentWindowDataRef 
		) noexcept;

		~DXContextState() = default;
	public:
		void SetCurrentShaderSet(DXShaderSetType shaderType) noexcept;
		void SetCurrentModelMatrix(const DirectX::XMMATRIX& modelMatrixRef) noexcept;
		void SetCurrentCameraData(const CMCameraData& cameraDataRef) noexcept;
		void SetCurrentCameraTransform(const CMCommon::CMRigidTransform& rigidTransformRef) noexcept;

		void UpdateResolution() noexcept;

		void UpdateCamera() noexcept;

		[[nodiscard]] DXShaderSetType CurrentShaderSet() const noexcept;
		[[nodiscard]] float CurrentAspectRatio() const noexcept;

		inline [[nodiscard]] DXCamera& Camera() noexcept { return m_Camera; }

		inline [[nodiscard]] const CMWindowData& CurrentWindowData() const noexcept { return m_CurrentWindowDataRef; }
		inline [[nodiscard]] RECT CurrentClientArea() const noexcept { return m_CurrentWindowDataRef.ClientArea; }
		inline [[nodiscard]] const DirectX::XMMATRIX& CurrentModelMatrix() const noexcept { return m_CurrentModelMatrix; }

		inline [[nodiscard]] bool IsModelMatrixSet() const noexcept { return m_ModelMatrixSet; }
		inline [[nodiscard]] bool CameraUpdated() const noexcept { return m_CameraUpdated; }
		inline [[nodiscard]] bool CameraTransformUpdated() const noexcept { return m_CameraTransformUpdated; }
		inline [[nodiscard]] bool WindowResized() const noexcept { return m_WindowResized; }
	private:
		CMCommon::CMLoggerWide& m_CMLoggerRef;
		DXShaderLibrary& m_ShaderLibraryRef;
		Components::DXDevice& m_DeviceRef;
		const CMWindowData& m_CurrentWindowDataRef;
		std::weak_ptr<IDXShaderSet> mP_CurrentShaderSet;
		DirectX::XMMATRIX m_CurrentModelMatrix = {};
		CMCameraData m_CameraData = {};
		DXCamera m_Camera;
		bool m_ModelMatrixSet = false;
		bool m_CameraUpdated = false;
		bool m_CameraTransformUpdated = false;
		bool m_WindowResized = false;
	};
	
	class DXContext
	{
		friend class CMRenderer;
	public:
		DXContext(CMCommon::CMLoggerWide& cmLoggerRef, const CMWindowData& currentWindowData) noexcept;
		~DXContext() noexcept;
	public:
		void Init(const HWND hWnd) noexcept;
		void Shutdown() noexcept;

		void SetShaderSet(DXShaderSetType setType) noexcept;
		void SetModelMatrix(const DirectX::XMMATRIX& modelMatrixRef) noexcept;
		void SetCamera(const CMCameraData& cameraDataRef) noexcept;
		void SetCameraTransform(const CMCommon::CMRigidTransform& rigidTransformRef) noexcept;

		void Clear(CMCommon::NormColor normColor) noexcept;
		void Present() noexcept;

		void DrawIndexed(const std::span<float> vertices, const std::span<uint16_t> indices, UINT vertexStride, UINT vertexOffset = 0) noexcept;

		//void TestDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY, float offsetZ) noexcept;
		//void TestTextureDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY, float offsetZ) noexcept;

		void ImGuiNewFrame() noexcept;

		void ImGuiBegin(std::string_view windowTitle, bool* pIsOpen = nullptr, ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None) noexcept;
		void ImGuiEnd() noexcept;

		bool ImGuiBeginChild(
			std::string_view stringID,
			ImVec2 size = ImVec2(0.0f, 0.0f),
			ImGuiChildFlags childFlags = ImGuiChildFlags_None,
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None
		) noexcept;

		bool ImGuiBeginChild(
			ImGuiID id,
			ImVec2 size = ImVec2(0.0f, 0.0f),
			ImGuiChildFlags childFlags = ImGuiChildFlags_None,
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None
		) noexcept;

		void ImGuiSlider(std::string_view label, float* pValue, float valueMin, float valueMax) noexcept;
		void ImGuiSliderAngle(std::string_view label, float* pRadians, float angleMin, float angleMax) noexcept;
		void ImGuiShowDemoWindow() noexcept;

		[[nodiscard]] bool ImGuiCollapsingHeader(std::string_view title, ImGuiTreeNodeFlags flags = 0) noexcept;

		void ImGuiEndChild() noexcept;
		void ImGuiEndFrame() noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }

		inline [[nodiscard]] DXShaderSetType CurrentShaderSet() const noexcept { return m_State.CurrentShaderSet(); }
		inline [[nodiscard]] Components::DXWriter& Writer() noexcept { return m_Writer; }
	private:
		void InitImGui(const HWND hWnd) noexcept;
		void ShutdownImGui() noexcept;

		void CreateRTV() noexcept;
		void BindRTV() noexcept;

		void SetViewport() noexcept;
		void SetTopology() noexcept;

		void ReleaseViews() noexcept;

		void OnWindowResize() noexcept;
	private:
		CMCommon::CMLoggerWide& m_CMLoggerRef;
		DXShaderLibrary m_ShaderLibrary;
		DXContextState m_State;
		Components::DXDevice m_Device;
		Components::DXFactory m_Factory;
		Components::DXSwapChain m_SwapChain;
		Components::DXWriter m_Writer;
		CM_IF_NDEBUG_REPLACE(
			HMODULE m_DebugInterfaceModule = nullptr;
			Microsoft::WRL::ComPtr<IDXGIDebug> mP_DebugInterface;
			Components::DXInfoQueue m_InfoQueue;
		);
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> mP_RasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mP_RTV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mP_DSV;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}