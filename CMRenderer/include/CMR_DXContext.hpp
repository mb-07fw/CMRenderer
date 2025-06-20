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
#include "CMR_DXResources.hpp"

namespace CMRenderer
{
	class CMRenderer; // Forward declare here for friend declaration in DXContext...
}

namespace CMRenderer::CMDirectX
{
	struct CMFrameData
	{
		float ResolutionX = 0.0f;
		float ResolutionY = 0.0f;
		float padding[2] = { 0.0f, 0.0f };
	};

	struct CMShaderConstants
	{
		static constexpr uint8_t S_CAMERA_TRANSFORM_REGISTER_SLOT = 0;
		static constexpr uint8_t S_FRAME_DATA_REGISTER_SLOT = 1;
	};

	struct CircleInstance
	{
		DirectX::XMMATRIX ModelMatrix = {};

		/* Automatically assumes a unit circle radius of 0.5f. */
		float Radius = 0.5f;
		CMCommon::CMFloat3 Padding;
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

		void RebindCurrentShaderSet() noexcept;

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

		template <typename VertexTy>
			requires std::is_trivially_copyable_v<VertexTy>
		inline void DrawIndexed(
			std::span<VertexTy> vertices,
			std::span<uint16_t> indices,
			UINT vertexStride = sizeof(VertexTy),
			UINT vertexOffset = 0u
		) noexcept;

		template <typename VertexTy, typename IndexTy, typename InstanceTy>
			requires std::is_trivially_copyable_v<VertexTy> &&
				std::is_trivially_copyable_v<IndexTy> &&
				std::is_trivially_copyable_v<InstanceTy>
		inline void DrawIndexedInstanced(
			std::span<VertexTy> vertices,
			std::span<IndexTy> indices,
			std::span<InstanceTy> instances,
			UINT totalVertices,
			UINT totalIndices,
			UINT totalInstances,
			UINT vertexStride = sizeof(VertexTy),
			UINT indexStride = sizeof(IndexTy),
			UINT instanceStride = sizeof(InstanceTy),
			UINT vertexOffset = 0u,
			UINT instanceOffset = 0u,
			UINT indexOffset = 0u,
			UINT verticesRegister = 0u,
			UINT instancesRegister = 1u,
			UINT indicesPerInstance = 0u,
			UINT instanceCount = 0u,
			UINT startIndexLocation = 0u,
			UINT baseVertexLocation = 0u,
			UINT startInstanceLocation = 0u
		) noexcept;

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

		[[nodiscard]] bool ImGuiCollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags = 0) noexcept;

		[[nodiscard]] bool ImGuiButton(std::string_view label, ImVec2 size = ImVec2(0, 0)) noexcept;

		void ImGuiShowDemoWindow() noexcept;

		void ImGuiEndFrame() noexcept;
		void ImGuiEndChild() noexcept;

		void ReportLiveObjects() noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }

		[[nodiscard]] bool IsFullscreen() noexcept;

		inline [[nodiscard]] DXShaderSetType CurrentShaderSet() const noexcept { return m_State.CurrentShaderSet(); }
		inline [[nodiscard]] Components::DXWriter& Writer() noexcept { return m_Writer; }
	private:
		void InitImGui(const HWND hWnd) noexcept;
		void ShutdownImGui() noexcept;

		void CreateRTV() noexcept;
		void BindRTV() noexcept;

		void SetViewport() noexcept;
		void SetTopology() noexcept;

		void ResetState() noexcept;

		void OnWindowResize() noexcept;

		void RebindCurrentShaderSet() noexcept;
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
		DXConstantBuffer m_CBFrameData;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};

	template <typename VertexTy>
		requires std::is_trivially_copyable_v<VertexTy>
	inline void DXContext::DrawIndexed(
		std::span<VertexTy> vertices,
		std::span<uint16_t> indices,
		UINT vertexStride,
		UINT vertexOffset
	) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"DXContext [DrawIndexed] | DXContext isn't initialized.");

		m_CMLoggerRef.LogFatalNLIf(vertices.data() == nullptr, L"DXContext [DrawIndexed] | Vertices data is nullptr.");
		m_CMLoggerRef.LogFatalNLIf(indices.data() == nullptr, L"DXContext [DrawIndexed] | Indices data is nullptr.");

		m_CMLoggerRef.LogFatalNLIf(vertices.size() == 0ull, L"DXContext [DrawIndexed] | Vertices size is 0.");
		m_CMLoggerRef.LogFatalNLIf(indices.size() == 0ull, L"DXContext [DrawIndexed] | Indices size is 0.");

		m_CMLoggerRef.LogFatalNLIf(vertexStride == 0u, L"DXContext [DrawIndexed] | Vertex stride is 0.");

		BindRTV();

		DXVertexBuffer vertexBuffer(vertexStride, vertices);
		DXIndexBuffer indexBuffer(DXGI_FORMAT_R16_UINT, indices);

		DirectX::XMMATRIX modelMatrix = {};
		if (m_State.IsModelMatrixSet())
			modelMatrix = m_State.CurrentModelMatrix();
		else
			modelMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

		if (m_State.CameraUpdated() || m_State.WindowResized())
			m_State.UpdateCamera();

		DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(modelMatrix * m_State.Camera().ViewProjectionMatrix());

		DXConstantBuffer mvpBuffer(CMShaderConstants::S_CAMERA_TRANSFORM_REGISTER_SLOT, std::span<DirectX::XMMATRIX>(&mvpMatrix, 1u));

		m_CMLoggerRef.LogFatalNLIf(FAILED(vertexBuffer.Create(m_Device)), L"DXContext [DrawIndexed] | Failed to create vertex buffer.");
		m_CMLoggerRef.LogFatalNLIf(FAILED(indexBuffer.Create(m_Device)), L"DXContext [DrawIndexed] | Failed to create index buffer.");
		m_CMLoggerRef.LogFatalNLIf(FAILED(mvpBuffer.Create(m_Device)), L"DXContext [DrawIndexed] | Failed to create mvp constant buffer.");

		vertexBuffer.Bind(m_Device);
		indexBuffer.Bind(m_Device);
		mvpBuffer.BindVS(m_Device);

		UINT indexCount = static_cast<UINT>(indices.size());

		m_Device.ContextRaw()->DrawIndexed(indexCount, 0, 0);

		vertexBuffer.Release();
		indexBuffer.Release();
		mvpBuffer.Release();

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_CMLoggerRef.LogFatalNL(L"DXContext [DrawIndexed] | Debug messages generated after drawing.");
			}
		);
	}

	template <typename VertexTy, typename IndexTy, typename InstanceTy>
		requires std::is_trivially_copyable_v<VertexTy> &&
			std::is_trivially_copyable_v<IndexTy> &&
			std::is_trivially_copyable_v<InstanceTy>
	inline void DXContext::DrawIndexedInstanced(
		std::span<VertexTy> vertices,
		std::span<IndexTy> indices,
		std::span<InstanceTy> instances,
		UINT totalVertices,
		UINT totalIndices,
		UINT totalInstances,
		UINT vertexStride,
		UINT indexStride,
		UINT instanceStride,
		UINT vertexOffset,
		UINT instanceOffset,
		UINT indexOffset,
		UINT verticesRegister,
		UINT instancesRegister,
		UINT indicesPerInstance,
		UINT instanceCount,
		UINT startIndexLocation,
		UINT baseVertexLocation,
		UINT startInstanceLocation
	) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"DXContext [DrawIndexedInstanced] | DXContext isn't initialized.");

		m_CMLoggerRef.LogFatalNLIf(vertices.data() == nullptr, L"DXContext [DrawIndexedInstanced] | Vertex data is nullptr.");
		m_CMLoggerRef.LogFatalNLIf(indices.data() == nullptr, L"DXContext [DrawIndexedInstanced] | Index data is nullptr.");
		m_CMLoggerRef.LogFatalNLIf(instances.data() == nullptr, L"DXContext [DrawIndexedInstanced] | Instance data is nullptr.");

		m_CMLoggerRef.LogFatalNLIf(vertices.size() == 0, L"DXContext [DrawIndexedInstanced] | Vertex data size is 0.");
		m_CMLoggerRef.LogFatalNLIf(indices.size() == 0, L"DXContext [DrawIndexedInstanced] | Index data size is 0.");
		m_CMLoggerRef.LogFatalNLIf(instances.size() == 0, L"DXContext [DrawIndexedInstanced] | Instance data size is 0.");

		m_CMLoggerRef.LogFatalNLIf(totalVertices == 0, L"DXContext [DrawIndexedInstanced] | Total vertices is 0.");
		m_CMLoggerRef.LogFatalNLIf(totalIndices == 0, L"DXContext [DrawIndexedInstanced] | Total indices is 0.");
		m_CMLoggerRef.LogFatalNLIf(totalInstances == 0, L"DXContext [DrawIndexedInstanced] | Total indices is 0.");

		m_CMLoggerRef.LogFatalNLIf(vertexStride == 0, L"DXContext [DrawIndexedInstanced] | Vertex stride is 0.");
		m_CMLoggerRef.LogFatalNLIf(indexStride == 0, L"DXContext [DrawIndexedInstanced] | Index stride is 0.");
		m_CMLoggerRef.LogFatalNLIf(instanceStride == 0, L"DXContext [DrawIndexedInstanced] | Instance stride is 0.");

		m_CMLoggerRef.LogFatalNLVariadicIf(
			verticesRegister == instancesRegister,
			L"DXContext [DrawIndexedInstanced] | Vertices buffer and instances buffer registers clash : ", 
			instancesRegister
		);

		m_CMLoggerRef.LogFatalNLVariadicIf(
			static_cast<size_t>(startIndexLocation) > totalIndices,
			L"DXContext [DrawIndexedInstanced] | Start index location is invalid. [0 - ",
			(totalIndices - 1), "] : ", startIndexLocation
		);

		m_CMLoggerRef.LogFatalNLVariadicIf(
			static_cast<size_t>(baseVertexLocation) > totalVertices,
			L"DXContext [DrawIndexedInstanced] | Base vertex location is invalid. [0 - ",
			(totalVertices - 1), "] : ", baseVertexLocation
		);

		m_CMLoggerRef.LogFatalNLVariadicIf(
			static_cast<size_t>(startInstanceLocation) > totalInstances,
			L"DXContext [DrawIndexedInstanced] | Start instance location is invalid. [0 - ",
			(totalInstances - 1), "] : ", startInstanceLocation
		);

		if (indicesPerInstance == 0u)
			indicesPerInstance = totalIndices;

		if (instanceCount == 0u)
			instanceCount = totalInstances;

		BindRTV();

		DXVertexBuffer verticesBuffer(vertexStride, vertices, verticesRegister);
		DXVertexBuffer instancesBuffer(instanceStride, instances, instancesRegister);
		DXIndexBuffer indicesBuffer(DXGI_FORMAT_R16_UINT, indices);

		DirectX::XMMATRIX modelMatrix = {};
		if (m_State.IsModelMatrixSet())
			modelMatrix = m_State.CurrentModelMatrix();
		else
			modelMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

		if (m_State.CameraUpdated() || m_State.WindowResized())
			m_State.UpdateCamera();

		DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(modelMatrix * m_State.Camera().ViewProjectionMatrix());

		DXConstantBuffer mvpBuffer(CMShaderConstants::S_CAMERA_TRANSFORM_REGISTER_SLOT, std::span<DirectX::XMMATRIX>(&mvpMatrix, 1u));

		m_CMLoggerRef.LogFatalNLIf(
			FAILED(verticesBuffer.Create(m_Device)), 
			L"DXContext [DrawIndexedInstanced] | Failed to create vertices buffer."
		);

		m_CMLoggerRef.LogFatalNLIf(
			FAILED(instancesBuffer.Create(m_Device)),
			L"DXContext [DrawIndexedInstanced] | Failed to create instances buffer."
		);

		m_CMLoggerRef.LogFatalNLIf(
			FAILED(indicesBuffer.Create(m_Device)),
			L"DXContext [DrawIndexedInstanced] | Failed to create indices buffer."
		);

		m_CMLoggerRef.LogFatalNLIf(
			FAILED(mvpBuffer.Create(m_Device)),
			L"DXContext [DrawIndexedInstanced] | Failed to create mvp buffer."
		);

		verticesBuffer.Bind(m_Device, vertexOffset);
		instancesBuffer.Bind(m_Device, instanceOffset);
		indicesBuffer.Bind(m_Device, indexOffset);
		mvpBuffer.BindVS(m_Device);

		m_Device.ContextRaw()->DrawIndexedInstanced(indicesPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_CMLoggerRef.LogFatalNL(L"DXContext [DrawIndexedInstanced] | Debug messages generated after drawing.");
			}
		);
	}
}