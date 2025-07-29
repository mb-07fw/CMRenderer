#pragma once

#include <d3d11.h>
#include <dxgidebug.h>
#include <wrl/client.h>

#include <imgui/imgui.h>

#include <memory>
#include <span>

#include "DX/DX11/DX11_Components.hpp"
#include "DX/DX11/DX11_ShaderLibrary.hpp"
#include "DX/DX11/DX11_Camera.hpp"
#include "DX/DX11/DX11_Resources.hpp"
#include "Win/Win_WindowSettings.hpp"
#include "Common/Macros.hpp"
#include "Common/Logger.hpp"
#include "Common/Types.hpp"

namespace CMEngine
{
	/* Forward declare here to friend declaration in Renderer.*/
	class CMEngine;
}

namespace CMEngine::DX::DX11
{
	template <typename... Args>
	constexpr bool AllTriviallyCopyable = (std::is_trivially_copyable_v<Args>&&...);

	struct FrameData
	{
		float ResolutionX = 0.0f;
		float ResolutionY = 0.0f;
		float padding[2] = { 0.0f, 0.0f };
	};

	struct ShaderConstants
	{
		static constexpr uint8_t S_CAMERA_TRANSFORM_REGISTER_SLOT = 0;
		static constexpr uint8_t S_FRAME_DATA_REGISTER_SLOT = 1;
	};

	struct CircleInstance
	{
		DirectX::XMMATRIX ModelMatrix = {};

		/* Automatically assumes a unit circle radius of 0.5f. */
		float Radius = 0.5f;
		Common::Float3 Padding;
	};

	struct DrawDescriptor
	{
		static constexpr uint32_t S_INVALID_SENTINEL = static_cast<UINT>(-1);
		static constexpr uint32_t S_INFER_FROM_CONTAINER = S_INVALID_SENTINEL;
		static constexpr uint32_t S_DEFAULT = 0;

		static constexpr uint32_t S_VERTEX_BUFFER_REGISTER_DEFAULT = 0;
		static constexpr uint32_t S_INSTANCE_BUFFER_REGISTER_DEFAULT = 1;

		/* <---- Universal Draw Call Parameters: ----> */
		uint32_t TotalVertices = S_INFER_FROM_CONTAINER;						// Total vertices to be submitted for rendering.
		uint32_t VertexByteStride = S_INFER_FROM_CONTAINER;						// Stride in bytes of each vertex in the vertex buffer.
		uint32_t VertexByteOffset = S_DEFAULT;									// Offset in bytes from the start of the vertex buffer to the first vertex to read.
		uint32_t VertexBufferRegister = S_VERTEX_BUFFER_REGISTER_DEFAULT;		// Register slot to bind vertex buffer to; see ID3D11DeviceContext::IASetVertexBuffers @StartSlot.

		/* <---- Indexed Draw Call Parameters: ---->  */
		uint32_t TotalIndices = S_INFER_FROM_CONTAINER;							// Total indices to be submitted.
		uint32_t IndexByteStride = S_INFER_FROM_CONTAINER;						// Stride in bytes of each index in the index buffer.
		uint32_t IndexByteOffset = S_DEFAULT;									// Offset in bytes from the start of the index buffer to the first index to read.
		uint32_t StartIndexLocation = S_DEFAULT;								// Index of first index to be read from the index buffer.
		uint32_t BaseVertexLocation = S_DEFAULT;								// A value added to each index before reading a vertex from the vertex buffer.

		/* <---- Instanced Draw Call Parameters: ----> */
		uint32_t TotalInstances = S_INFER_FROM_CONTAINER;						// Total instances to be submitted.
		uint32_t InstanceByteStride = S_INFER_FROM_CONTAINER;					// Stride in bytes of each instance in the instance buffer.
		uint32_t InstanceByteOffset = S_DEFAULT;								// Offset in bytes from the start of the instance buffer to the first instance to read.
		uint32_t InstanceBufferRegister = S_INSTANCE_BUFFER_REGISTER_DEFAULT;   // Register slot to bind instance buffer to; see ID3D11DeviceContext::IASetVertexBuffers @StartSlot.
		uint32_t StartInstanceLocation = S_DEFAULT;								// A value added to each index before reading an instance from the instance buffer.

		/* <---- IndexedInstanced Draw Call Parameters: ----> */
		uint32_t IndicesPerInstance = S_INFER_FROM_CONTAINER;					// Number of indices read from the index buffer for each instance.						   
	};

	class RendererState
	{
	public:
		RendererState(
			Common::LoggerWide& logger,
			ShaderLibrary& shaderLibrary,
			Device& device,
			const Win::WindowData& currentWindowData
		) noexcept;

		~RendererState() = default;
	public:
		void SetCurrentShaderSet(ShaderSetType shaderType) noexcept;
		void SetCurrentCameraData(const Core::CameraData& cameraData) noexcept;
		void SetCurrentCameraTransform(const Common::RigidTransform& rigidTransform) noexcept;

		void RebindCurrentShaderSet() noexcept;

		void FlagResize() noexcept;
		void UpdateCamera() noexcept;

		[[nodiscard]] ShaderSetType CurrentShaderSet() const noexcept;
		[[nodiscard]] float CurrentAspectRatio() const noexcept;

		inline [[nodiscard]] Camera& Camera() noexcept { return m_Camera; }

		inline [[nodiscard]] const Win::WindowData& CurrentWindowData() const noexcept { return m_CurrentWindowData; }
		inline [[nodiscard]] RECT CurrentClientArea() const noexcept { return m_CurrentWindowData.ClientArea; }

		inline [[nodiscard]] bool CameraUpdated() const noexcept { return m_CameraUpdated; }
		inline [[nodiscard]] bool CameraTransformUpdated() const noexcept { return m_CameraTransformUpdated; }
		inline [[nodiscard]] bool WindowResized() const noexcept { return m_WindowResized; }
	private:
		Common::LoggerWide& m_Logger;
		ShaderLibrary& m_ShaderLibrary;
		Device& m_Device;
		const Win::WindowData& m_CurrentWindowData;
		std::weak_ptr<IShaderSet> mP_CurrentShaderSet;
		Core::CameraData m_CameraData = {};
		DX11::Camera m_Camera;
		bool m_CameraUpdated = false;
		bool m_CameraTransformUpdated = false;
		bool m_WindowResized = false;
	};
	
	class Renderer
	{
		friend class CMEngine;
	public:
		Renderer(Common::LoggerWide& logger, const Win::WindowData& currentWindowData) noexcept;
		~Renderer() noexcept;
	public:
		void Init(const HWND hWnd) noexcept;
		void Shutdown() noexcept;

		void SetShaderSet(ShaderSetType setType) noexcept;
		void CacheCamera(const Core::CameraData& cameraData) noexcept;
		void CacheCameraTransform(const Common::RigidTransform& rigidTransform) noexcept;

		void Clear(Common::NormColor normColor) noexcept;
		void Present() noexcept;

		void DrawIndexed(
			std::span<const std::byte> vertices,
			std::span<const std::byte> indices,
			const Common::Transform& modelTransform,
			const DrawDescriptor& descriptor
		) noexcept;

		//template <typename VertexTy, typename IndexTy>
		//	requires AllTriviallyCopyable<VertexTy, IndexTy>
		//inline void DrawIndexed(
		//	std::span<const VertexTy> vertices,
		//	std::span<const IndexTy> indices,
		//	const Common::Transform& modelTransform,
		//	DrawDescriptor& descriptor // NOTE: Will be modified if any fields are inferred.
		//) noexcept;

		//template <typename VertexTy, typename IndexTy, typename InstanceTy>
		//	requires AllTriviallyCopyable<VertexTy, IndexTy, InstanceTy>
		//inline void DrawIndexedInstanced(
		//	std::span<const VertexTy> vertices,
		//	std::span<const IndexTy> indices,
		//	std::span<const InstanceTy> instances,
		//	DrawDescriptor& descriptor // NOTE: Will be modified if any fields are inferred.
		//) noexcept;

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

		template <typename... Args>
		void ImGuiText(std::string_view fmt, Args&&... args) noexcept;

		void ImGuiSlider(std::string_view label, float* pValue, float valueMin, float valueMax) noexcept;
		void ImGuiSliderAngle(std::string_view label, float* pRadians, float angleMin, float angleMax) noexcept;

		[[nodiscard]] bool ImGuiCollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags = 0) noexcept;

		[[nodiscard]] bool ImGuiButton(std::string_view label, ImVec2 size = ImVec2(0, 0)) noexcept;

		void ImGuiShowDemoWindow() noexcept;

		void ImGuiEndFrame() noexcept;
		void ImGuiEndChild() noexcept;

		void ReportLiveObjects() noexcept;

		[[nodiscard]] Common::Rect CurrentArea() const noexcept;
		[[nodiscard]] Common::Float2 CurrentResolution() const noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }

		[[nodiscard]] bool IsFullscreen() noexcept;

		inline [[nodiscard]] ShaderSetType CurrentShaderSet() const noexcept { return m_State.CurrentShaderSet(); }
		//inline [[nodiscard]] Writer& Writer() noexcept { return m_Writer; }
	private:
		void EnforceValidDrawDescriptor(
			std::span<const std::byte> vertices,
			const DrawDescriptor descriptor
		) noexcept;

		void EnforceValidDrawDescriptorIndexed(
			std::span<const std::byte> vertices,
			std::span<const std::byte> indices,
			const DrawDescriptor& descriptor
		) noexcept;

		template <typename VertexTy, typename IndexTy, typename InstanceTy>
			requires AllTriviallyCopyable<VertexTy, IndexTy, InstanceTy>
		void EnforceValidDrawDescriptorIndexedInstanced(
			std::span<VertexTy> vertices,
			std::span<IndexTy> indices,
			std::span<InstanceTy> instances,
			DrawDescriptor& descriptor
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

		inline constexpr [[nodiscard]] DXGI_FORMAT BytesToDXGIFormat(uint32_t bytes) const noexcept;
	private:
		Common::LoggerWide& m_Logger;
		ShaderLibrary m_ShaderLibrary;
		RendererState m_State;
		Device m_Device;
		Factory m_Factory;
		SwapChain m_SwapChain;
		//Writer m_Writer;
		CM_IF_NDEBUG_REPLACE(
			HMODULE mP_DebugInterfaceModule = nullptr;
			Microsoft::WRL::ComPtr<IDXGIDebug> mP_DebugInterface;
			InfoQueue m_InfoQueue;
		);
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> mP_RasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mP_RTV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mP_DSV;
		ConstantBuffer m_CBFrameData;
		Common::Transform m_CachedModelTransform;
		DirectX::XMMATRIX m_CachedModelMatrix;
		bool m_ModelTransformSet = false;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};

	//template <typename VertexTy, typename IndexTy>
	//	requires AllTriviallyCopyable<VertexTy, IndexTy>
	//inline void Renderer::DrawIndexed(
	//	std::span<const VertexTy> vertices,
	//	std::span<const IndexTy> indices,
	//	const Common::Transform& modelTransform,
	//	DrawDescriptor& descriptor
	//) noexcept
	//{
	//	/* TODO: Rework Draw Call system. (Tidy up calls under a single base Draw() function)
	//	 *		 Update model matrices if @modelTransform differs from previous model transfrom. (Base Draw() function)
	//	 */

	//	constexpr std::wstring_view FuncTag = L"Renderer [DrawIndexed] | ";

	//	m_Logger.LogFatalNLIf(
	//		!m_Initialized,
	//		L"Renderer [DrawIndexed] | Renderer isn't initialized."
	//	);

	//	m_Logger.LogFatalNLIf(
	//		vertices.data() == nullptr,
	//		L"Renderer [DrawIndexed] | Vertices data is nullptr."
	//	);

	//	m_Logger.LogFatalNLIf(
	//		indices.data() == nullptr,
	//		L"Renderer [DrawIndexed] | Indices data is nullptr."
	//	);

	//	m_Logger.LogFatalNLIf(
	//		vertices.size() == 0, 
	//		L"Renderer [DrawIndexed] | Vertices size is 0."
	//	);

	//	m_Logger.LogFatalNLIf(
	//		indices.size() == 0,
	//		L"Renderer [DrawIndexed] | Indices size is 0."
	//	);

	//	EnforceValidDrawDescriptorIndexed(vertices, indices, descriptor);

	//	BindRTV();

	//	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

	//	if (descriptor.IndexByteStride == sizeof(uint8_t))
	//		format = DXGI_FORMAT_R8_UINT;
	//	else if (descriptor.IndexByteStride == sizeof(uint16_t))
	//		format = DXGI_FORMAT_R16_UINT;

	//	m_Logger.LogFatalNLFormattedIf(
	//		format == DXGI_FORMAT_UNKNOWN,
	//		FuncTag,
	//		L"Index type format is unsupported. Stride : `{}`.",
	//		descriptor.IndexByteStride
	//	);

	//	DirectX::XMMATRIX modelMatrix = {};
	//	if (m_State.IsModelMatrixSet())
	//		modelMatrix = m_State.CurrentModelMatrix();
	//	else
	//		modelMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	//	if (m_State.CameraUpdated() || m_State.WindowResized())
	//		m_State.UpdateCamera();

	//	DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(
	//		modelMatrix * 
	//		m_State.Camera().ViewProjectionMatrix()
	//	);

	//	VertexBuffer vertexBuffer(descriptor.VertexByteStride, vertices);
	//	IndexBuffer indexBuffer(format, indices);
	//	ConstantBuffer mvpBuffer(
	//		ShaderConstants::S_CAMERA_TRANSFORM_REGISTER_SLOT,
	//		std::span<DirectX::XMMATRIX>(&mvpMatrix, 1u)
	//	);

	//	m_Logger.LogFatalNLIf(
	//		FAILED(vertexBuffer.Create(m_Device)),
	//		L"Renderer [DrawIndexed] | Failed to create vertex buffer."
	//	);

	//	m_Logger.LogFatalNLIf(
	//		FAILED(indexBuffer.Create(m_Device)),
	//		L"Renderer [DrawIndexed] | Failed to create index buffer."
	//	);

	//	m_Logger.LogFatalNLIf(
	//		FAILED(mvpBuffer.Create(m_Device)),
	//		L"Renderer [DrawIndexed] | Failed to create mvp constant buffer."
	//	);

	//	vertexBuffer.Bind(m_Device, descriptor.VertexByteOffset);
	//	indexBuffer.Bind(m_Device, descriptor.IndexByteOffset);
	//	mvpBuffer.BindVS(m_Device);

	//	m_Device.ContextRaw()->DrawIndexed(descriptor.TotalIndices, 0, 0);

	//	vertexBuffer.Release();
	//	indexBuffer.Release();
	//	mvpBuffer.Release();

	//	CM_IF_DEBUG(
	//		if (!m_InfoQueue.IsQueueEmpty())
	//		{
	//			m_InfoQueue.LogMessages();
	//			m_Logger.LogFatalNL(L"Renderer [DrawIndexed] | Debug messages generated after drawing.");
	//		}
	//	);
	//}

	//template <typename VertexTy, typename IndexTy, typename InstanceTy>
	//	requires AllTriviallyCopyable<VertexTy, IndexTy, InstanceTy>
	//inline void Renderer::DrawIndexedInstanced(
	//	std::span<const VertexTy> vertices,
	//	std::span<const IndexTy> indices,
	//	std::span<const InstanceTy> instances,
	//	DrawDescriptor& descriptor // NOTE: Will be modified if any fields are inferred.
	//) noexcept
	//{
	//	constexpr std::wstring_view FuncTag = L"Renderer [DrawIndexedInstanced] | ";

	//	m_Logger.LogFatalNLTaggedIf(!m_Initialized, FuncTag, L"Renderer isn't initialized.");

	//	m_Logger.LogFatalNLTaggedIf(vertices.data() == nullptr, FuncTag, L"Vertex data is nullptr.");
	//	m_Logger.LogFatalNLTaggedIf(indices.data() == nullptr, FuncTag, L"Index data is nullptr.");
	//	m_Logger.LogFatalNLTaggedIf(instances.data() == nullptr, FuncTag, L"Instance data is nullptr.");
	//	
	//	EnforceValidDrawDescriptorIndexedInstanced(vertices, indices, instances, descriptor);

	//	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	//	if (descriptor.IndexByteStride == sizeof(uint8_t))
	//		format = DXGI_FORMAT_R8_UINT;
	//	else if (descriptor.IndexByteStride == sizeof(uint16_t))
	//		format = DXGI_FORMAT_R16_UINT;

	//	m_Logger.LogFatalNLFormattedIf(
	//		format == DXGI_FORMAT_UNKNOWN,
	//		FuncTag,
	//		L"Index type format is unsupported. Stride : `{}`.",
	//		descriptor.IndexByteStride
	//	);

	//	BindRTV();

	//	DirectX::XMMATRIX modelMatrix = {};
	//	if (m_State.IsModelMatrixSet())
	//		modelMatrix = m_State.CurrentModelMatrix();
	//	else
	//		modelMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	//	if (m_State.CameraUpdated() || m_State.WindowResized())
	//		m_State.UpdateCamera();

	//	DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(modelMatrix * m_State.Camera().ViewProjectionMatrix());

	//	VertexBuffer verticesBuffer(
	//		descriptor.VertexByteStride,
	//		vertices,
	//		descriptor.VertexBufferRegister
	//	);

	//	VertexBuffer instancesBuffer(
	//		descriptor.InstanceByteStride,
	//		instances, 
	//		descriptor.InstanceBufferRegister
	//	);

	//	IndexBuffer indicesBuffer(format, indices);
	//	ConstantBuffer mvpBuffer(
	//		ShaderConstants::S_CAMERA_TRANSFORM_REGISTER_SLOT,
	//		std::span<DirectX::XMMATRIX>(&mvpMatrix, 1u)
	//	);

	//	m_Logger.LogFatalNLIf(
	//		FAILED(verticesBuffer.Create(m_Device)), 
	//		L"Renderer [DrawIndexedInstanced] | Failed to create vertices buffer."
	//	);

	//	m_Logger.LogFatalNLIf(
	//		FAILED(instancesBuffer.Create(m_Device)),
	//		L"Renderer [DrawIndexedInstanced] | Failed to create instances buffer."
	//	);

	//	m_Logger.LogFatalNLIf(
	//		FAILED(indicesBuffer.Create(m_Device)),
	//		L"Renderer [DrawIndexedInstanced] | Failed to create indices buffer."
	//	);

	//	m_Logger.LogFatalNLIf(
	//		FAILED(mvpBuffer.Create(m_Device)),
	//		L"Renderer [DrawIndexedInstanced] | Failed to create mvp buffer."
	//	);

	//	verticesBuffer.Bind(m_Device, descriptor.VertexByteOffset);
	//	instancesBuffer.Bind(m_Device, descriptor.InstanceByteOffset);
	//	indicesBuffer.Bind(m_Device, descriptor.IndexByteOffset);
	//	mvpBuffer.BindVS(m_Device);

	//	m_Device.ContextRaw()->DrawIndexedInstanced(
	//		descriptor.IndicesPerInstance,
	//		descriptor.TotalInstances,
	//		descriptor.StartIndexLocation,
	//		descriptor.BaseVertexLocation,
	//		descriptor.StartInstanceLocation
	//	);

	//	CM_IF_DEBUG(
	//		if (!m_InfoQueue.IsQueueEmpty())
	//		{
	//			m_InfoQueue.LogMessages();
	//			m_Logger.LogFatalNL(L"Renderer [DrawIndexedInstanced] | Debug messages generated after drawing.");
	//		}
	//	);
	//}

	template <typename... Args>
	void Renderer::ImGuiText(std::string_view fmt, Args&&... args) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiBeginChild] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::Text(fmt.data(), std::forward<Args>(args)...);
	}

	template <typename VertexTy, typename IndexTy, typename InstanceTy>
		requires AllTriviallyCopyable<VertexTy, IndexTy, InstanceTy>
	void Renderer::EnforceValidDrawDescriptorIndexedInstanced(
		std::span<VertexTy> vertices,
		std::span<IndexTy> indices,
		std::span<InstanceTy> instances,
		DrawDescriptor& descriptor // NOTE: Will be modified if any fields are inferred.
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"Renderer [EnforceValidDrawDescriptorIndexedInstanced] | ";

		EnforceValidDrawDescriptorIndexed(vertices, indices, descriptor);

		if (descriptor.TotalInstances == DrawDescriptor::S_INFER_FROM_CONTAINER)
			descriptor.TotalInstances = static_cast<UINT>(instances.size());
		if (descriptor.InstanceByteStride == DrawDescriptor::S_INFER_FROM_CONTAINER)
			descriptor.InstanceByteStride = sizeof(InstanceTy);

		if (descriptor.IndicesPerInstance == DrawDescriptor::S_INFER_FROM_CONTAINER)
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

		constexpr UINT MaxRegisterSlot = VertexBuffer::MaxRegisterSlot();

		m_Logger.LogFatalNLFormattedIf(
			!VertexBuffer::IsValidRegister(descriptor.InstanceBufferRegister),
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

	inline constexpr [[nodiscard]] DXGI_FORMAT Renderer::BytesToDXGIFormat(uint32_t bytes) const noexcept
	{
		switch (bytes)
		{
			case sizeof(uint8_t):	return DXGI_FORMAT_R8_UINT;
			case sizeof(uint16_t):  return DXGI_FORMAT_R16_UINT;
			default:				return DXGI_FORMAT_UNKNOWN;
		}
	}
}