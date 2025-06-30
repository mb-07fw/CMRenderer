#pragma once

#include <d3d11.h>
#include <dxgidebug.h>

#include <wrl/client.h>
#include <DirectXMath.h>

#include <imgui/imgui.h>

#include <memory>
#include <span>

#include "DirectX/CME_DXComponents.hpp"
#include "DirectX/CME_DXShaderLibrary.hpp"
#include "DirectX/CME_DXCamera.hpp"
#include "DirectX/CME_DXResources.hpp"
#include "Core/CME_WindowSettings.hpp"
#include "CMC_Macros.hpp"
#include "CMC_Logger.hpp"
#include "CMC_Types.hpp"

namespace CMEngine
{
	class CMEngine;
}

namespace CMEngine::DirectXAPI::DX11
{
	template <typename... Args>
	constexpr bool AllTriviallyCopyable = (std::is_trivially_copyable_v<Args>&&...);

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

	struct CMCircleInstance
	{
		DirectX::XMMATRIX ModelMatrix = {};

		/* Automatically assumes a unit circle radius of 0.5f. */
		float Radius = 0.5f;
		CMCommon::CMFloat3 Padding;
	};

	struct DXDrawDescriptor
	{
		static constexpr UINT S_INVALID_SENTINEL = static_cast<UINT>(-1);
		static constexpr UINT S_INFER_FROM_CONTAINER = S_INVALID_SENTINEL;
		static constexpr UINT S_DEFAULT = 0;

		static constexpr UINT S_VERTEX_BUFFER_REGISTER_DEFAULT = 0;
		static constexpr UINT S_INSTANCE_BUFFER_REGISTER_DEFAULT = 1;

		/* <---- Universal Draw Call Parameters: ----> */
		UINT TotalVertices = S_INFER_FROM_CONTAINER;						// Total vertices to be submitted for rendering.
		UINT VertexByteStride = S_INFER_FROM_CONTAINER;						// Stride in bytes of each vertex in the vertex buffer.
		UINT VertexByteOffset = S_DEFAULT;									// Offset in bytes from the start of the vertex buffer to the first vertex to read.
		UINT VertexBufferRegister = S_VERTEX_BUFFER_REGISTER_DEFAULT;		// Register slot to bind vertex buffer to; see ID3D11DeviceContext::IASetVertexBuffers @StartSlot.

		/* <---- Indexed Draw Call Parameters: ---->  */
		UINT TotalIndices = S_INFER_FROM_CONTAINER;							// Total indices to be submitted.
		UINT IndexByteStride = S_INFER_FROM_CONTAINER;						// Stride in bytes of each index in the index buffer.
		UINT IndexByteOffset = S_DEFAULT;									// Offset in bytes from the start of the index buffer to the first index to read.
		UINT StartIndexLocation = S_DEFAULT;								// Index of first index to be read from the index buffer.
		UINT BaseVertexLocation = S_DEFAULT;								// A value added to each index before reading a vertex from the vertex buffer.

		/* <---- Instanced Draw Call Parameters: ----> */
		UINT TotalInstances = S_INFER_FROM_CONTAINER;						// Total instances to be submitted.
		UINT InstanceByteStride = S_INFER_FROM_CONTAINER;					// Stride in bytes of each instance in the instance buffer.
		UINT InstanceByteOffset = S_DEFAULT;								// Offset in bytes from the start of the instance buffer to the first instance to read.
		UINT InstanceBufferRegister = S_INSTANCE_BUFFER_REGISTER_DEFAULT;   // Register slot to bind instance buffer to; see ID3D11DeviceContext::IASetVertexBuffers @StartSlot.
		UINT StartInstanceLocation = S_DEFAULT;								// A value added to each index before reading an instance from the instance buffer.

		/* <---- IndexedInstanced Draw Call Parameters: ----> */
		UINT IndicesPerInstance = S_INFER_FROM_CONTAINER;					// Number of indices read from the index buffer for each instance.						   
	};

	class DXRendererState
	{
	public:
		DXRendererState(
			CMCommon::CMLoggerWide& logger,
			DXShaderLibrary& shaderLibrary,
			DXDevice& device,
			const CMWindowData& currentWindowData
		) noexcept;

		~DXRendererState() = default;
	public:
		void SetCurrentShaderSet(DXShaderSetType shaderType) noexcept;
		void SetCurrentModelMatrix(const DirectX::XMMATRIX& modelMatrix) noexcept;
		void SetCurrentCameraData(const CMCameraData& cameraData) noexcept;
		void SetCurrentCameraTransform(const CMCommon::CMRigidTransform& rigidTransform) noexcept;

		void RebindCurrentShaderSet() noexcept;

		void FlagResize() noexcept;
		void UpdateCamera() noexcept;

		[[nodiscard]] DXShaderSetType CurrentShaderSet() const noexcept;
		[[nodiscard]] float CurrentAspectRatio() const noexcept;

		inline [[nodiscard]] DXCamera& Camera() noexcept { return m_Camera; }

		inline [[nodiscard]] const CMWindowData& CurrentWindowData() const noexcept { return m_CurrentWindowData; }
		inline [[nodiscard]] RECT CurrentClientArea() const noexcept { return m_CurrentWindowData.ClientArea; }
		inline [[nodiscard]] const DirectX::XMMATRIX& CurrentModelMatrix() const noexcept { return m_CurrentModelMatrix; }

		inline [[nodiscard]] bool IsModelMatrixSet() const noexcept { return m_ModelMatrixSet; }
		inline [[nodiscard]] bool CameraUpdated() const noexcept { return m_CameraUpdated; }
		inline [[nodiscard]] bool CameraTransformUpdated() const noexcept { return m_CameraTransformUpdated; }
		inline [[nodiscard]] bool WindowResized() const noexcept { return m_WindowResized; }
	private:
		CMCommon::CMLoggerWide& m_Logger;
		DXShaderLibrary& m_ShaderLibrary;
		DXDevice& m_Device;
		const CMWindowData& m_CurrentWindowData;
		std::weak_ptr<IDXShaderSet> mP_CurrentShaderSet;
		DirectX::XMMATRIX m_CurrentModelMatrix = {};
		CMCameraData m_CameraData = {};
		DXCamera m_Camera;
		bool m_ModelMatrixSet = false;
		bool m_CameraUpdated = false;
		bool m_CameraTransformUpdated = false;
		bool m_WindowResized = false;
	};
	
	class DXRenderer
	{
		friend class CMEngine;
	public:
		DXRenderer(CMCommon::CMLoggerWide& logger, const CMWindowData& currentWindowData) noexcept;
		~DXRenderer() noexcept;
	public:
		void Init(const HWND hWnd) noexcept;
		void Shutdown() noexcept;

		void SetShaderSet(DXShaderSetType setType) noexcept;
		void SetModelMatrix(const DirectX::XMMATRIX& modelMatrix) noexcept;
		void SetCamera(const CMCameraData& cameraData) noexcept;
		void SetCameraTransform(const CMCommon::CMRigidTransform& rigidTransform) noexcept;

		void Clear(CMCommon::NormColor normColor) noexcept;
		void Present() noexcept;

		template <typename VertexTy, typename IndexTy>
			requires AllTriviallyCopyable<VertexTy, IndexTy>
		inline void DrawIndexed(
			std::span<const VertexTy> vertices,
			std::span<const IndexTy> indices,
			DXDrawDescriptor& descriptor // NOTE: Will be modified if any fields are inferred.
		) noexcept;

		template <typename VertexTy, typename IndexTy, typename InstanceTy>
			requires AllTriviallyCopyable<VertexTy, IndexTy, InstanceTy>
		inline void DrawIndexedInstanced(
			std::span<const VertexTy> vertices,
			std::span<const IndexTy> indices,
			std::span<const InstanceTy> instances,
			DXDrawDescriptor& descriptor // NOTE: Will be modified if any fields are inferred.
		) noexcept;

		//void TestDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY, float offsetZ) noexcept;
		//void TestTextureDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY, float offsetZ) noexcept;

		void ImGuiNewFrame() noexcept;

		void ImGuiBegin(
			std::string_view windowTitle,
			bool* pIsOpen = nullptr,
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None
		) noexcept;

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

		[[nodiscard]] CMCommon::CMRect CurrentRenderArea() const noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }

		[[nodiscard]] bool IsFullscreen() noexcept;

		inline [[nodiscard]] DXShaderSetType CurrentShaderSet() const noexcept { return m_State.CurrentShaderSet(); }
		//inline [[nodiscard]] DXWriter& Writer() noexcept { return m_Writer; }
	private:
		template <typename VertexTy>
			requires AllTriviallyCopyable<VertexTy>
		void EnforceValidDrawDescriptor(std::span<VertexTy> vertices, DXDrawDescriptor& descriptor) noexcept;

		template <typename VertexTy, typename IndexTy>
			requires AllTriviallyCopyable<VertexTy, IndexTy>
		void EnforceValidDrawDescriptorIndexed(std::span<VertexTy> vertices, std::span<IndexTy> indices, DXDrawDescriptor& descriptor) noexcept;

		template <typename VertexTy, typename IndexTy, typename InstanceTy>
			requires AllTriviallyCopyable<VertexTy, IndexTy, InstanceTy>
		void EnforceValidDrawDescriptorIndexedInstanced(
			std::span<VertexTy> vertices,
			std::span<IndexTy> indices,
			std::span<InstanceTy> instances,
			DXDrawDescriptor& descriptor
		) noexcept;

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
		CMCommon::CMLoggerWide& m_Logger;
		DXShaderLibrary m_ShaderLibrary;
		DXRendererState m_State;
		DXDevice m_Device;
		DXFactory m_Factory;
		DXSwapChain m_SwapChain;
		//DXWriter m_Writer;
		CM_IF_NDEBUG_REPLACE(
			HMODULE mP_DebugInterfaceModule = nullptr;
			Microsoft::WRL::ComPtr<IDXGIDebug> mP_DebugInterface;
			DXInfoQueue m_InfoQueue;
		);
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> mP_RasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mP_RTV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mP_DSV;
		DXConstantBuffer m_CBFrameData;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};

	template <typename VertexTy, typename IndexTy>
		requires AllTriviallyCopyable<VertexTy, IndexTy>
	inline void DXRenderer::DrawIndexed(
		std::span<const VertexTy> vertices,
		std::span<const IndexTy> indices,
		DXDrawDescriptor& descriptor
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"DXRenderer [DrawIndexed] | ";

		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [DrawIndexed] | DXRenderer isn't initialized."
		);

		m_Logger.LogFatalNLIf(
			vertices.data() == nullptr,
			L"DXRenderer [DrawIndexed] | Vertices data is nullptr."
		);

		m_Logger.LogFatalNLIf(
			indices.data() == nullptr,
			L"DXRenderer [DrawIndexed] | Indices data is nullptr."
		);

		m_Logger.LogFatalNLIf(
			vertices.size() == 0, 
			L"DXRenderer [DrawIndexed] | Vertices size is 0."
		);

		m_Logger.LogFatalNLIf(
			indices.size() == 0,
			L"DXRenderer [DrawIndexed] | Indices size is 0."
		);

		EnforceValidDrawDescriptorIndexed(vertices, indices, descriptor);

		BindRTV();

		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		if (descriptor.IndexByteStride == sizeof(uint8_t))
			format = DXGI_FORMAT_R8_UINT;
		else if (descriptor.IndexByteStride == sizeof(uint16_t))
			format = DXGI_FORMAT_R16_UINT;

		m_Logger.LogFatalNLFormattedIf(
			format == DXGI_FORMAT_UNKNOWN,
			FuncTag,
			L"Index type format is unsupported. Stride : `{}`.",
			descriptor.IndexByteStride
		);

		DirectX::XMMATRIX modelMatrix = {};
		if (m_State.IsModelMatrixSet())
			modelMatrix = m_State.CurrentModelMatrix();
		else
			modelMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

		if (m_State.CameraUpdated() || m_State.WindowResized())
			m_State.UpdateCamera();

		DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(
			modelMatrix * 
			m_State.Camera().ViewProjectionMatrix()
		);

		DXVertexBuffer vertexBuffer(descriptor.VertexByteStride, vertices);
		DXIndexBuffer indexBuffer(format, indices);
		DXConstantBuffer mvpBuffer(
			CMShaderConstants::S_CAMERA_TRANSFORM_REGISTER_SLOT,
			std::span<DirectX::XMMATRIX>(&mvpMatrix, 1u)
		);

		m_Logger.LogFatalNLIf(
			FAILED(vertexBuffer.Create(m_Device)),
			L"DXRenderer [DrawIndexed] | Failed to create vertex buffer."
		);

		m_Logger.LogFatalNLIf(
			FAILED(indexBuffer.Create(m_Device)),
			L"DXRenderer [DrawIndexed] | Failed to create index buffer."
		);

		m_Logger.LogFatalNLIf(
			FAILED(mvpBuffer.Create(m_Device)),
			L"DXRenderer [DrawIndexed] | Failed to create mvp constant buffer."
		);

		vertexBuffer.Bind(m_Device, descriptor.VertexByteOffset);
		indexBuffer.Bind(m_Device, descriptor.IndexByteOffset);
		mvpBuffer.BindVS(m_Device);

		m_Device.ContextRaw()->DrawIndexed(descriptor.TotalIndices, 0, 0);

		vertexBuffer.Release();
		indexBuffer.Release();
		mvpBuffer.Release();

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_Logger.LogFatalNL(L"DXRenderer [DrawIndexed] | Debug messages generated after drawing.");
			}
		);
	}

	template <typename VertexTy, typename IndexTy, typename InstanceTy>
		requires AllTriviallyCopyable<VertexTy, IndexTy, InstanceTy>
	inline void DXRenderer::DrawIndexedInstanced(
		std::span<const VertexTy> vertices,
		std::span<const IndexTy> indices,
		std::span<const InstanceTy> instances,
		DXDrawDescriptor& descriptor // NOTE: Will be modified if any fields are inferred.
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"DXRenderer [DrawIndexedInstanced] | ";

		m_Logger.LogFatalNLTaggedIf(!m_Initialized, FuncTag, L"DXRenderer isn't initialized.");

		m_Logger.LogFatalNLTaggedIf(vertices.data() == nullptr, FuncTag, L"Vertex data is nullptr.");
		m_Logger.LogFatalNLTaggedIf(indices.data() == nullptr, FuncTag, L"Index data is nullptr.");
		m_Logger.LogFatalNLTaggedIf(instances.data() == nullptr, FuncTag, L"Instance data is nullptr.");
		
		EnforceValidDrawDescriptorIndexedInstanced(vertices, indices, instances, descriptor);

		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		if (descriptor.IndexByteStride == sizeof(uint8_t))
			format = DXGI_FORMAT_R8_UINT;
		else if (descriptor.IndexByteStride == sizeof(uint16_t))
			format = DXGI_FORMAT_R16_UINT;

		m_Logger.LogFatalNLFormattedIf(
			format == DXGI_FORMAT_UNKNOWN,
			FuncTag,
			L"Index type format is unsupported. Stride : `{}`.",
			descriptor.IndexByteStride
		);

		BindRTV();

		DirectX::XMMATRIX modelMatrix = {};
		if (m_State.IsModelMatrixSet())
			modelMatrix = m_State.CurrentModelMatrix();
		else
			modelMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

		if (m_State.CameraUpdated() || m_State.WindowResized())
			m_State.UpdateCamera();

		DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(modelMatrix * m_State.Camera().ViewProjectionMatrix());

		DXVertexBuffer verticesBuffer(
			descriptor.VertexByteStride,
			vertices,
			descriptor.VertexBufferRegister
		);

		DXVertexBuffer instancesBuffer(
			descriptor.InstanceByteStride,
			instances, 
			descriptor.InstanceBufferRegister
		);

		DXIndexBuffer indicesBuffer(format, indices);
		DXConstantBuffer mvpBuffer(
			CMShaderConstants::S_CAMERA_TRANSFORM_REGISTER_SLOT,
			std::span<DirectX::XMMATRIX>(&mvpMatrix, 1u)
		);

		m_Logger.LogFatalNLIf(
			FAILED(verticesBuffer.Create(m_Device)), 
			L"DXRenderer [DrawIndexedInstanced] | Failed to create vertices buffer."
		);

		m_Logger.LogFatalNLIf(
			FAILED(instancesBuffer.Create(m_Device)),
			L"DXRenderer [DrawIndexedInstanced] | Failed to create instances buffer."
		);

		m_Logger.LogFatalNLIf(
			FAILED(indicesBuffer.Create(m_Device)),
			L"DXRenderer [DrawIndexedInstanced] | Failed to create indices buffer."
		);

		m_Logger.LogFatalNLIf(
			FAILED(mvpBuffer.Create(m_Device)),
			L"DXRenderer [DrawIndexedInstanced] | Failed to create mvp buffer."
		);

		verticesBuffer.Bind(m_Device, descriptor.VertexByteOffset);
		instancesBuffer.Bind(m_Device, descriptor.InstanceByteOffset);
		indicesBuffer.Bind(m_Device, descriptor.IndexByteOffset);
		mvpBuffer.BindVS(m_Device);

		m_Device.ContextRaw()->DrawIndexedInstanced(
			descriptor.IndicesPerInstance,
			descriptor.TotalInstances,
			descriptor.StartIndexLocation,
			descriptor.BaseVertexLocation,
			descriptor.StartInstanceLocation
		);

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_Logger.LogFatalNL(L"DXRenderer [DrawIndexedInstanced] | Debug messages generated after drawing.");
			}
		);
	}

	template <typename VertexTy>
		requires AllTriviallyCopyable<VertexTy>
	void DXRenderer::EnforceValidDrawDescriptor(std::span<VertexTy> vertices, DXDrawDescriptor& descriptor) noexcept
	{
		constexpr std::wstring_view FuncTag = L"DXRenderer [EnforceValidDrawDescriptor] | ";

		if (descriptor.TotalVertices == DXDrawDescriptor::S_INFER_FROM_CONTAINER)
			descriptor.TotalVertices = static_cast<UINT>(vertices.size());
		if (descriptor.VertexByteStride == DXDrawDescriptor::S_INFER_FROM_CONTAINER)
			descriptor.VertexByteStride = sizeof(VertexTy);

		UINT verticesSize = static_cast<UINT>(vertices.size());
		UINT verticesSizeBytes = static_cast<UINT>(vertices.size_bytes());

		m_Logger.LogFatalNLTaggedIf(
			descriptor.TotalVertices == 0,
			FuncTag,
			L"TotalVertices should not be zero."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.TotalVertices > verticesSize,
			FuncTag,
			L"TotalVertices exceeds size of vertex buffer. "
			L"Size : `{}`, TotalVertices : `{}`.",
			verticesSize, descriptor.TotalVertices
		);

		m_Logger.LogFatalNLTaggedIf(
			descriptor.VertexByteStride == 0,
			FuncTag,
			L"VertexByteStride should not be zero."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.VertexByteStride > verticesSizeBytes,
			FuncTag,
			L"VertexByteStride exceeds byte size of vertex buffer. "
			L"Size Bytes : `{}`, VertexByteStride : `{}`.",
			verticesSizeBytes, descriptor.VertexByteStride
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.VertexByteOffset > verticesSizeBytes,
			FuncTag,
			L"VertexByteOffset exceeds byte size of vertex buffer. "
			L"Size Bytes : `{}`, VertexByteOffset : `{}`.",
			verticesSizeBytes, descriptor.VertexByteOffset
		);

		constexpr UINT MaxRegisterSlot = DXVertexBuffer::MaxRegisterSlot();

		m_Logger.LogFatalNLFormattedIf(
			!DXVertexBuffer::IsValidRegister(descriptor.VertexBufferRegister),
			FuncTag,
			L"VertexBufferRegister is invalid : `{}`. "
			L"Must be within [0 - {}]",
			descriptor.VertexBufferRegister, MaxRegisterSlot
		);
	}

	template <typename VertexTy, typename IndexTy>
		requires AllTriviallyCopyable<VertexTy, IndexTy>
	void DXRenderer::EnforceValidDrawDescriptorIndexed(std::span<VertexTy> vertices, std::span<IndexTy> indices, DXDrawDescriptor& descriptor) noexcept
	{
		constexpr std::wstring_view FuncTag = L"DXRenderer [EnforceValidDrawDescriptorIndexed] | ";

		EnforceValidDrawDescriptor(vertices, descriptor);

		if (descriptor.TotalIndices == DXDrawDescriptor::S_INFER_FROM_CONTAINER)
			descriptor.TotalIndices = static_cast<UINT>(indices.size());
		if (descriptor.IndexByteStride == DXDrawDescriptor::S_INFER_FROM_CONTAINER)
			descriptor.IndexByteStride = sizeof(IndexTy);

		UINT indicesSize = static_cast<UINT>(indices.size());
		UINT indicesSizeBytes = static_cast<UINT>(indices.size_bytes());

		m_Logger.LogFatalNLTaggedIf(
			descriptor.TotalIndices == 0,
			FuncTag,
			L"TotalIndices should not be zero."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.TotalIndices > indicesSize,
			FuncTag,
			L"TotalIndices exceeds size of index buffer. "
			L"Size : `{}`, TotalIndices : `{}`.",
			indicesSize, descriptor.TotalIndices
		);

		m_Logger.LogFatalNLTaggedIf(
			descriptor.IndexByteStride == 0,
			FuncTag,
			L"IndexByteStride should not be zero."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.IndexByteStride > indicesSizeBytes,
			FuncTag,
			L"IndexByteStride exceeds byte size of index buffer. "
			L"Byte Size : `{}`, IndexByteStride : `{}`.",
			indicesSizeBytes, descriptor.IndexByteStride
		);

		bool isValidIndexStride = descriptor.IndexByteStride == sizeof(uint8_t) ||
			descriptor.IndexByteStride == sizeof(uint16_t);

		m_Logger.LogFatalNLFormattedIf(
			!isValidIndexStride,
			FuncTag,
			L"IndexByteStride doesn't equal a supported byte size (uint8_t, or uint16_t). "
			L"Stride : `{}`.",
			descriptor.IndexByteStride
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.IndexByteOffset > indicesSizeBytes,
			FuncTag,
			L"IndexByteOffset exceeds byte size of vertex buffer. "
			L"Byte Size : `{}`, IndexByteOffset : `{}`.",
			indicesSizeBytes, descriptor.IndexByteOffset
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.StartIndexLocation > descriptor.TotalIndices,
			FuncTag,
			L"StartIndexLocation exceeds TotalIndices."
			L"StartIndexLocation : `{}`, TotalIndices : `{}`.",
			descriptor.StartIndexLocation, descriptor.TotalIndices
		);
	}

	template <typename VertexTy, typename IndexTy, typename InstanceTy>
		requires AllTriviallyCopyable<VertexTy, IndexTy, InstanceTy>
	void DXRenderer::EnforceValidDrawDescriptorIndexedInstanced(
		std::span<VertexTy> vertices,
		std::span<IndexTy> indices,
		std::span<InstanceTy> instances,
		DXDrawDescriptor& descriptor // NOTE: Will be modified if any fields are inferred.
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"DXRenderer [EnforceValidDrawDescriptorIndexedInstanced] | ";

		EnforceValidDrawDescriptorIndexed(vertices, indices, descriptor);

		if (descriptor.TotalInstances == DXDrawDescriptor::S_INFER_FROM_CONTAINER)
			descriptor.TotalInstances = static_cast<UINT>(instances.size());
		if (descriptor.InstanceByteStride == DXDrawDescriptor::S_INFER_FROM_CONTAINER)
			descriptor.InstanceByteStride = sizeof(InstanceTy);

		if (descriptor.IndicesPerInstance == DXDrawDescriptor::S_INFER_FROM_CONTAINER)
			descriptor.IndicesPerInstance = descriptor.TotalIndices;

		UINT instancesSize = static_cast<UINT>(instances.size());
		UINT instancesSizeBytes = static_cast<UINT>(instances.size_bytes());

		m_Logger.LogFatalNLTaggedIf(
			descriptor.TotalInstances == 0,
			FuncTag,
			L"TotalInstances should not be 0."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.TotalInstances > instancesSize,
			FuncTag,
			L"TotalInstances exceeds size of instance buffer. "
			L"TotalInstances : `{}`, Size : `{}`.",
			descriptor.TotalInstances, instancesSize
		);

		m_Logger.LogFatalNLTaggedIf(
			descriptor.InstanceByteStride == 0,
			FuncTag,
			L"InstanceByteStride should not be 0."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.InstanceByteStride > instancesSizeBytes,
			FuncTag,
			L"InstanceByteStride exceeds byte size of instance buffer. "
			L"InstanceByteStride : `{}`, Byte Size : `{}`.",
			descriptor.InstanceByteStride, instancesSizeBytes
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.InstanceByteOffset > instancesSizeBytes,
			FuncTag,
			L"InstanceByteOffset exceeds byte size of instance buffer. "
			L"InstanceByteOffset : `{}`, Byte Size : `{}`.",
			descriptor.InstanceByteOffset, instancesSizeBytes
		);

		constexpr UINT MaxRegisterSlot = DXVertexBuffer::MaxRegisterSlot();

		m_Logger.LogFatalNLFormattedIf(
			!DXVertexBuffer::IsValidRegister(descriptor.InstanceBufferRegister),
			FuncTag,
			L"InstanceBufferRegister is invalid : `{}`. "
			L"Must be within [0 - {}]",
			descriptor.InstanceBufferRegister, MaxRegisterSlot
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.VertexBufferRegister == descriptor.InstanceBufferRegister,
			FuncTag,
			L"VertexBufferRegister and InstanceBufferRegister clash. "
			L"Two vertex buffers should not have the same register slot : `{}`.",
			descriptor.VertexBufferRegister
		);
	}
}