#include "Core/PCH.hpp"
#include "DX/DX11/DX11_Renderer.hpp"
#include "Win/Win_Utility.hpp"
#include "Common/Utility.hpp"
#include "Common/Macros.hpp"

namespace CMEngine::DX::DX11
{
#pragma region RendererState
	RendererState::RendererState(
		Common::LoggerWide& logger,
		ShaderLibrary& shaderLibrary,
		Device& device,
		const Win::WindowData& currentWindowData
	) noexcept
		: m_ShaderLibrary(shaderLibrary), 
		  m_Device(device),
		  m_CurrentWindowData(currentWindowData),
		  m_Camera(),
		  m_Logger(logger)
	{
	}

	void RendererState::SetCurrentShaderSet(ShaderSetType shaderType) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Device.IsCreated(),
			L"RendererState [SetCurrentShaderSet] | Device is not created."
		);

		m_Logger.LogFatalNLIf(
			!m_ShaderLibrary.IsInitialized(),
			L"RendererState [SetCurrentShaderSet] | Shader library is not initialized."
		);

		std::shared_ptr<IShaderSet> pCurrentShaderSet = nullptr;
		if (!mP_CurrentShaderSet.expired())
		{
			pCurrentShaderSet = mP_CurrentShaderSet.lock();

			if (m_Logger.LogWarningNLIf(pCurrentShaderSet->Type == shaderType, L"RendererState [SetCurrentShaderSet] | ShaderSetType is already set."))
				return;
		}

		mP_CurrentShaderSet = m_ShaderLibrary.GetSetOfType(shaderType);

		m_Logger.LogFatalNLIf(
			mP_CurrentShaderSet.expired(),
			L"RendererState [SetCurrentShaderSet] | Retrieved shader set pointer is expired."
		);

		if (pCurrentShaderSet == nullptr)
			pCurrentShaderSet = mP_CurrentShaderSet.lock();

		m_Logger.LogFatalNLIf(
			!pCurrentShaderSet->IsCreated,
			L"RendererState [SetCurrentShaderSet] | Retrieved shader set wasn't created previously."
		);

		m_Logger.LogFatalNLIf(
			pCurrentShaderSet->pVertexShader.Get() == nullptr,
			L"RendererState [SetCurrentShaderSet] | Current shader set's vertex shader was nullptr."
		);

		m_Logger.LogFatalNLIf(
			pCurrentShaderSet->pPixelShader.Get() == nullptr,
			L"RendererState [SetCurrentShaderSet] | Current shader set's pixel shader was nullptr."
		);

		m_Logger.LogFatalNLIf(
			pCurrentShaderSet->pInputLayout.Get() == nullptr,
			L"RendererState [SetCurrentShaderSet] | Current shader set's input layout was nullptr."
		);

		m_Device.ContextRaw()->VSSetShader(pCurrentShaderSet->pVertexShader.Get(), nullptr, 0);
		m_Device.ContextRaw()->PSSetShader(pCurrentShaderSet->pPixelShader.Get(), nullptr, 0);
		m_Device.ContextRaw()->IASetInputLayout(pCurrentShaderSet->pInputLayout.Get());
	}

	void RendererState::SetCurrentCameraData(const Core::CameraData& cameraData) noexcept
	{
		m_CameraData = cameraData;
		m_CameraUpdated = true;
	}

	void RendererState::SetCurrentCameraTransform(const Common::RigidTransform& rigidTransform) noexcept
	{
		m_CameraData.RigidTransform = rigidTransform;
		m_CameraTransformUpdated = true;
		m_CameraUpdated = true;
	}

	void RendererState::SetCurrentCameraProjection(Core::PerspectiveParams perspectiveParams) noexcept
	{
		m_CameraData.Projection = perspectiveParams;
		m_CameraUpdated = true;
	}

	void RendererState::SetCurrentCameraProjection(const Core::OrthographicParams orthographicParams) noexcept
	{
		m_CameraData.Projection = orthographicParams;
		m_CameraUpdated = true;
	}

	void RendererState::RebindCurrentShaderSet() noexcept
	{
		m_Logger.LogFatalNLIf(
			mP_CurrentShaderSet.expired(),
			L"RendererState [RebindCurrentShaderSet] | No shader set is currently set."
		);

		ShaderSetType currentSet = mP_CurrentShaderSet.lock()->Type;

		mP_CurrentShaderSet.reset();

		SetCurrentShaderSet(currentSet);
	}

	void RendererState::FlagResize() noexcept
	{
		if (CurrentAspectRatio() == m_Camera.AspectRatio())
			return;
		
		m_WindowResized = true;
	}
	
	void RendererState::UpdateCamera() noexcept
	{
		if (!m_CameraUpdated && !m_WindowResized)
			return;

		/* Camera updated and window resized, View, Projection and ViewProjection matrices need to be recalculated. */
		if (m_CameraUpdated && m_WindowResized)
			m_Camera.SetAll(m_CameraData, CurrentAspectRatio());
		/* Camera transform updated, only View and ViewProjection matrices need to be recalculated. */
		else if (m_CameraTransformUpdated)
			m_Camera.SetTransform(m_CameraData.RigidTransform);
		/* Camera projection updated, only Projection and ViewProjection matrices need to be recalculated. */
		else if (m_CameraProjectionUpdated)
			m_Camera.SetProjection(m_CameraData.Projection);
		/* Window resolution updated, only Projection and ViewProjection matrices need to be recalculated. */
		else if (m_WindowResized)
			m_Camera.SetAspectRatio(CurrentAspectRatio());

		m_CameraUpdated = false;
		m_CameraTransformUpdated = false;
		m_CameraProjectionUpdated = false;
		m_WindowResized = false;
	}

	[[nodiscard]] ShaderSetType RendererState::CurrentShaderSet() const noexcept
	{
		if (mP_CurrentShaderSet.expired())
			return ShaderSetType::INVALID;

		std::shared_ptr<IShaderSet> pCurrentShaderSet = mP_CurrentShaderSet.lock();

		return pCurrentShaderSet.get() == nullptr ? ShaderSetType::INVALID :
			pCurrentShaderSet->Type;
	}

	[[nodiscard]] float RendererState::CurrentAspectRatio() const noexcept
	{
		return static_cast<float>(m_CurrentWindowData.ClientArea.right) /
			m_CurrentWindowData.ClientArea.bottom;
	}
#pragma endregion

#pragma region Renderer

#define COMMA ,

	Renderer::Renderer(Common::LoggerWide& logger, const Win::WindowData& currentWindowData) noexcept
		: m_Logger(logger),
		  m_ShaderLibrary(logger),
		  m_State(logger, m_ShaderLibrary, m_Device, currentWindowData),
		  m_Device(logger),
		  m_Factory(logger),
		  m_SwapChain(logger),
		  //m_Writer(logger),
		  m_CBFrameData(ShaderConstants::S_FRAME_DATA_REGISTER_SLOT, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE) CM_IF_NDEBUG_REPLACE(COMMA)
		  CM_IF_NDEBUG_REPLACE(m_InfoQueue(logger))
	{
		CM_IF_DEBUG(
			mP_DebugInterfaceModule = LoadLibrary(L"Dxgidebug.dll");
		
			m_Logger.LogFatalNLIf(mP_DebugInterfaceModule == nullptr, L"Renderer [()] | Failed to load Dxgidebug.dll");

			// Here so Intelisense doesn't yell at me for de-referencing a nullptr, even though fatal logs terminate the program.
			if (mP_DebugInterfaceModule == nullptr)
				return;

			typedef HRESULT(WINAPI* DXGIGetDebugInterfaceFunc)(const IID&, void**);

			// Retrieve the address of DXGIGetDebugInterface function.
			DXGIGetDebugInterfaceFunc pDXGIGetDebugInterface =
				(DXGIGetDebugInterfaceFunc)GetProcAddress(mP_DebugInterfaceModule, "DXGIGetDebugInterface");

			m_Logger.LogFatalNLIf(pDXGIGetDebugInterface == nullptr, L"Renderer [()] | Failed to get function address for the DXGIGetDebugInterface.");

			// Here so Intelisense doesn't yell at me for de-referencing a nullptr, even though fatal logs terminate the program.
			if (pDXGIGetDebugInterface == nullptr)
				return;

			HRESULT hResult = pDXGIGetDebugInterface(IID_PPV_ARGS(&mP_DebugInterface));

			m_Logger.LogFatalNLIf(
				FAILED(hResult),
				L"Renderer [()] | Failed to retrieve a DXGI debug interface."
			);
		);

		m_Logger.LogInfoNL(L"Renderer [()] | Constructed.");
	}

	Renderer::~Renderer() noexcept
	{
		if (m_Initialized)
			Shutdown();

		CM_IF_DEBUG(FreeLibrary(mP_DebugInterfaceModule));

		m_Logger.LogInfoNL(L"Renderer [~()] | Destroyed.");
	}

	void Renderer::Init(const HWND hWnd) noexcept
	{
		m_Logger.LogWarningNLIf(
			m_Initialized,
			L"Renderer [Init] | Initializion has been attempted after RenderContext has already been initialized."
		);

		m_Device.Create();
		m_Factory.Create(m_Device);
		m_SwapChain.Create(hWnd, m_State.CurrentClientArea(), m_Factory, m_Device);
		//m_Writer.Create(m_SwapChain);

		CM_IF_DEBUG(
			m_InfoQueue.Create(m_Device);

			m_Logger.LogFatalNLIf(
				!m_InfoQueue.IsCreated(),
				L"Renderer [Init] | Failed to initialize info queue."
			);
		);

		InitImGui(hWnd);

		m_ShaderLibrary.Init(m_Device);

		FrameData frameData = {
			static_cast<float>(m_State.CurrentClientArea().right),
			static_cast<float>(m_State.CurrentClientArea().bottom),
			{ 0.0f, 0.0f }
		};

		m_CBFrameData.SetData(std::span(&frameData, 1u));

		m_Logger.LogFatalNLIf(
			FAILED(m_CBFrameData.Create(m_Device)),
			L"Renderer [Init] | Failed to create FrameData constant buffer."
		);

		m_CBFrameData.BindPS(m_Device);

		m_Factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		CreateRTV();
		SetViewport();
		SetTopology();

		m_Logger.LogInfoNL(L"Renderer [Init] | Initialized.");

		m_Initialized = true;
		m_Shutdown = false;
	}

	void Renderer::Shutdown() noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Shutdown, 
			L"Renderer [Shutdown] | Shutdown has been attempted after shutdown has already occured previously."
		);

		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [Shutdown] | Shutdown has been attempted before initialization."
		);

		ShutdownImGui();

		mP_RTV.Reset();
		m_Factory.Release();
		m_Device.Release();
		m_SwapChain.Release();
		//m_Writer.Release();

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
				m_InfoQueue.LogMessages();

			m_InfoQueue.Release()
		);

		m_ShaderLibrary.Shutdown();

		m_Initialized = false;
		m_Shutdown = true;
	}

	void Renderer::SetShaderSet(ShaderSetType setType) noexcept
	{
		m_Logger.LogFatalNLIf(
			setType == ShaderSetType::INVALID,
			L"Renderer [SetShaderSet] | The provided ShaderSetType should not be ShaderSetType::INVALID."
		);

		if (m_State.CurrentShaderSet() == setType)
		{
			m_Logger.LogWarningNL(L"Renderer [SetShaderSet] | The provided ShaderSetType is already set.");
			return;
		}

		m_State.SetCurrentShaderSet(setType);
	}

	void Renderer::SetCamera(const Core::CameraData& cameraData) noexcept
	{
		m_State.SetCurrentCameraData(cameraData);
	}

	void Renderer::SetCameraTransform(const Common::RigidTransform& rigidTransform) noexcept
	{
		m_State.SetCurrentCameraTransform(rigidTransform);
	}

	void Renderer::SetCameraProjection(Core::PerspectiveParams perspectiveParams) noexcept
	{
		m_State.SetCurrentCameraProjection(perspectiveParams);
	}

	void Renderer::SetCameraProjection(const Core::OrthographicParams& orthographicParams) noexcept
	{
		m_State.SetCurrentCameraProjection(orthographicParams);
	}

	void Renderer::CacheModelTransform(const Common::Transform& modelTransform) noexcept
	{
		if (m_CachedModelTransform.IsNearEqual(modelTransform))
			return;

		m_CachedModelTransform = modelTransform;
		m_CachedModelMatrix = DirectX::XMMatrixIdentity();

		if (!modelTransform.Scaling.IsEqual(Common::Float3(1.0f, 1.0f, 1.0f)))
			m_CachedModelMatrix *= DirectX::XMMatrixScaling(
				modelTransform.Scaling.x,
				modelTransform.Scaling.y,
				modelTransform.Scaling.z
			);

		if (!modelTransform.Rotation.IsZero())
			m_CachedModelMatrix *= DirectX::XMMatrixRotationRollPitchYaw(
				modelTransform.Rotation.x,
				modelTransform.Rotation.y,
				modelTransform.Rotation.z
			);

		if (!modelTransform.Translation.IsZero())
			m_CachedModelMatrix *= DirectX::XMMatrixTranslation(
				modelTransform.Translation.x,
				modelTransform.Translation.y,
				modelTransform.Translation.z
			);
	}

	void Renderer::Clear(Common::NormColor normColor) noexcept
	{
		m_Device.ContextRaw()->ClearRenderTargetView(mP_RTV.Get(), normColor.rgba);
		m_Device.ContextRaw()->ClearDepthStencilView(mP_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

		ImGuiNewFrame();
	}

	void Renderer::Present() noexcept
	{
		ImGuiEndFrame();

		HRESULT hResult = m_SwapChain->Present(1, 0);

		if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
					);

			m_Logger.LogFatalNLAppend(
				L"Renderer [Present] | Device error : ",
				Win::Utility::TranslateError(hResult)
			);
		}
		else if (FAILED(hResult))
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
					);

			m_Logger.LogFatalNLAppend(
				L"Renderer [Present] | Present error : ",
				Win::Utility::TranslateError(hResult)
			);
		}
	}

#pragma region Draw Calls
	void Renderer::DrawIndexed(
		std::span<const std::byte> vertices,
		std::span<const std::byte> indices,
		const Common::Transform& modelTransform,
		const DrawDescriptor& descriptor
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"Renderer [DrawIndexed] | ";

		EnforceValidDrawDescriptorIndexed(vertices, indices, descriptor);

		BindRTV();

		m_State.UpdateCamera();

		/* TODO: Only re-calculate mvp if camera or model transform get's updated. */
		DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(
			m_CachedModelMatrix * m_State.Camera().ViewProjectionMatrix()
		);

		constexpr uint8_t MVPRegisterSlot = ShaderConstants::S_CAMERA_TRANSFORM_REGISTER_SLOT;
		std::span<const DirectX::XMMATRIX> mvpMatrixSpan = Common::Utility::AsSpan(mvpMatrix);

		DXGI_FORMAT indexByteFormat = BytesToDXGIFormat(descriptor.IndexByteStride);

		VertexBuffer vertexBuffer(descriptor.VertexByteStride, vertices, descriptor.VertexBufferRegister);
		IndexBuffer indexBuffer(indexByteFormat, indices);
		ConstantBuffer mvpBuffer(MVPRegisterSlot, mvpMatrixSpan);
		
		m_Logger.LogFatalNLTaggedIf(
			FAILED(vertexBuffer.Create(m_Device)),
			FuncTag,
			L"Failed to create vertex buffer."
		);

		m_Logger.LogFatalNLTaggedIf(
			FAILED(indexBuffer.Create(m_Device)),
			FuncTag,
			L"Failed to create index buffer."
		);

		m_Logger.LogFatalNLTaggedIf(
			FAILED(mvpBuffer.Create(m_Device)),
			FuncTag,
			L"Failed to create constant buffer for MVP matrix."
		);

		vertexBuffer.Bind(m_Device, descriptor.VertexByteOffset);
		indexBuffer.Bind(m_Device, descriptor.IndexByteOffset);
		mvpBuffer.BindVS(m_Device);

		m_Device.ContextRaw()->DrawIndexed(descriptor.TotalIndices, 0, 0);

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_Logger.LogFatalNLTagged(FuncTag, L"Debug messages generated after drawing.");
			}
		);
	}

	void Renderer::DrawIndexedInstanced(
		std::span<const std::byte> vertices,
		std::span<const std::byte> indices,
		std::span<const std::byte> instances,
		const DrawDescriptor& descriptor
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"Renderer [DrawIndexed] | ";

		EnforceValidDrawDescriptorIndexedInstanced(vertices, indices, instances, descriptor);

		m_State.UpdateCamera();

		BindRTV();

		/* TODO: Only update upload mvpBuffer if it changes. */
		DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(
			m_CachedModelMatrix * m_State.Camera().ViewProjectionMatrix()
		);

		constexpr uint8_t MVPRegisterSlot = ShaderConstants::S_CAMERA_TRANSFORM_REGISTER_SLOT;
		std::span<const DirectX::XMMATRIX> mvpMatrixSpan = Common::Utility::AsSpan(mvpMatrix);

		DXGI_FORMAT indexByteFormat = BytesToDXGIFormat(descriptor.IndexByteStride);

		VertexBuffer vertexBuffer(descriptor.VertexByteStride, vertices, descriptor.VertexBufferRegister);
		VertexBuffer instanceBuffer(descriptor.InstanceByteStride, instances, descriptor.InstanceBufferRegister);
		IndexBuffer indexBuffer(indexByteFormat, indices);
		ConstantBuffer mvpBuffer(MVPRegisterSlot, mvpMatrixSpan);

		m_Logger.LogFatalNLTaggedIf(
			FAILED(vertexBuffer.Create(m_Device)),
			FuncTag,
			L"Failed to create vertex buffer."
		);

		m_Logger.LogFatalNLTaggedIf(
			FAILED(instanceBuffer.Create(m_Device)),
			FuncTag,
			L"Failed to create instance buffer."
		);

		m_Logger.LogFatalNLTaggedIf(
			FAILED(indexBuffer.Create(m_Device)),
			FuncTag,
			L"Failed to create index buffer."
		);

		m_Logger.LogFatalNLTaggedIf(
			FAILED(mvpBuffer.Create(m_Device)),
			FuncTag,
			L"Failed to create constant buffer for MVP matrix."
		);

		vertexBuffer.Bind(m_Device, descriptor.VertexByteOffset);
		instanceBuffer.Bind(m_Device, descriptor.InstanceByteOffset);
		indexBuffer.Bind(m_Device, descriptor.IndexByteOffset);
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
				m_Logger.LogFatalNLTagged(FuncTag, L"Debug messages generated after drawing.");
			}
		);
	}
#pragma endregion

#pragma region ImGui Wrappers
	void Renderer::ImGuiNewFrame() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiNewFrame] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void Renderer::ImGuiBegin(std::string_view windowTitle, bool* pIsOpen, ImGuiWindowFlags windowFlags) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiBegin] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::Begin(windowTitle.data(), pIsOpen, windowFlags);
	}

	void Renderer::ImGuiEnd() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiEnd] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::End();
	}

	bool Renderer::ImGuiBeginChild(
		std::string_view stringID,
		ImVec2 size,
		ImGuiChildFlags childFlags,
		ImGuiWindowFlags windowFlags
	) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiBeginChild] Attempted to perform ImGui operation before context was initialized."
		);
	
		return ImGui::BeginChild(stringID.data(), size, childFlags, windowFlags);
	}

	bool Renderer::ImGuiBeginChild(
		ImGuiID id,
		ImVec2 size,
		ImGuiChildFlags childFlags,
		ImGuiWindowFlags windowFlags
	) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiBeginChild] Attempted to perform ImGui operation before context was initialized."
		);

		return ImGui::BeginChild(id, size, childFlags, windowFlags);
	}

	void Renderer::ImGuiSlider(std::string_view label, float* pValue, float valueMin, float valueMax) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiSlider] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::SliderFloat(label.data(), pValue, valueMin, valueMax);
	}

	void Renderer::ImGuiSliderAngle(std::string_view label, float* pRadians, float angleMin, float angleMax) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiSliderAngle] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::SliderAngle(label.data(), pRadians, angleMin, angleMax);
	}

	void Renderer::ImGuiShowDemoWindow() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiShowDemoWindow] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::ShowDemoWindow();
	}

	[[nodiscard]] bool Renderer::ImGuiCollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiCollapsingHeader] Attempted to perform ImGui operation before context was initialized."
		);

		return ImGui::CollapsingHeader(label.data(), flags);
	}

	[[nodiscard]] bool Renderer::ImGuiButton(std::string_view label, ImVec2 size) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiButton] Attempted to perform ImGui operation before context was initialized."
		);

		return ImGui::Button(label.data(), size);
	}

	void Renderer::ImGuiEndFrame() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [ImGuiEndFrame] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void Renderer::ImGuiEndChild() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized, 
			L"Renderer [ImGuiEndChild] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::EndChild();
	}
#pragma endregion

	void Renderer::ReportLiveObjects() noexcept
	{
		CM_IF_DEBUG(
			HRESULT hResult = mP_DebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

			m_Logger.LogFatalNLIf(
				FAILED(hResult),
				L"Renderer [DrawIndexed] | Failed to report live objects."
			)
		);
	}

	[[nodiscard]] Common::Rect Renderer::CurrentArea() const noexcept
	{
		RECT clientArea = m_State.CurrentClientArea();

		return Common::Rect(
			static_cast<float>(clientArea.left),
			static_cast<float>(clientArea.right),
			static_cast<float>(clientArea.top),
			static_cast<float>(clientArea.bottom)
		);
	}

	[[nodiscard]] Common::Float2 Renderer::CurrentResolution() const noexcept
	{
		RECT clientArea = m_State.CurrentClientArea();

		return Common::Float2(
			static_cast<float>(clientArea.right),
			static_cast<float>(clientArea.bottom)
		);
	}

	[[nodiscard]] bool Renderer::IsFullscreen() noexcept
	{
		BOOL isFullscreen = 0;

		HRESULT hResult = m_SwapChain->GetFullscreenState(&isFullscreen, nullptr);

		m_Logger.LogFatalNLIf(
			FAILED(hResult),
			L"Renderer [IsFullscreen] | Failed to get fullscreen state."
		);

		return static_cast<bool>(isFullscreen);
	}

#pragma region EnforceDrawDescriptor Functions
	void Renderer::EnforceValidDrawDescriptor(
		std::span<const std::byte> vertices,
		const DrawDescriptor descriptor
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"Renderer [EnforceValidDrawDescriptor] |";

		size_t verticesSize = vertices.size();

		bool vertexDataMalformed = verticesSize == 0 ||
			vertices.data() == nullptr;

		m_Logger.LogFatalNLTaggedIf(
			vertexDataMalformed,
			FuncTag,
			L"Vertex data is malformed."
		);

		constexpr uint32_t MaxVBRegisterSlot = VertexBuffer::MaxRegisterSlot();

		m_Logger.LogFatalNLFormattedIf(
			!VertexBuffer::IsValidRegister(descriptor.VertexBufferRegister),
			FuncTag,
			L"`VertexBufferRegister` is invalid: `{}`. Expected range: [0, {}].",
			descriptor.VertexBufferRegister,
			MaxVBRegisterSlot
		);

		m_Logger.LogFatalNLTaggedIf(
			descriptor.TotalVertices == 0,
			FuncTag,
			L"`TotalVertices` is 0."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.TotalVertices > vertices.size(),
			FuncTag,
			L"`TotalVertices` is greater than the provided vertex data's size in bytes. Total: `{}`. Data Size: `{}`.",
			descriptor.TotalVertices, verticesSize
		);

		m_Logger.LogFatalNLTaggedIf(
			descriptor.VertexByteStride == 0,
			FuncTag,
			L"`VertexByteStride` is 0."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.VertexByteStride > vertices.size(),
			FuncTag,
			L"`VertexByteStride` exceeds provided vertex data byte size. Stride: `{}`. Data Size: `{}`.",
			descriptor.VertexByteStride, verticesSize
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.VertexByteOffset >= vertices.size(),
			FuncTag,
			L"`VertexByteOffset` exceeds or reaches provided vertex data byte size. Offset: `{}`. Data Size: `{}`.",
			descriptor.VertexByteOffset, verticesSize
		);
	}

	void Renderer::EnforceValidDrawDescriptorIndexed(
		std::span<const std::byte> vertices,
		std::span<const std::byte> indices,
		const DrawDescriptor& descriptor
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"Renderer [EnforceValidDrawDescriptorIndexed] |";

		EnforceValidDrawDescriptor(vertices, descriptor);

		size_t indicesSize = indices.size();

		bool indexDataMalformed = indicesSize == 0 ||
			indices.data() == nullptr;

		m_Logger.LogFatalNLTaggedIf(
			indexDataMalformed,
			FuncTag,
			L"Index data is malformed."
		);
		
		m_Logger.LogFatalNLTaggedIf(
			descriptor.TotalIndices == 0,
			FuncTag,
			L"`TotalIndices` is 0."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.TotalIndices > indices.size(),
			FuncTag,
			L"`TotalIndices` is greater than the provided index data's size in bytes. Total: `{}`. Data Size: `{}`.",
			descriptor.TotalIndices, indicesSize
		);

		m_Logger.LogFatalNLTaggedIf(
			descriptor.IndexByteStride == 0,
			FuncTag,
			L"`IndexByteStride` is 0."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.IndexByteStride > indices.size(),
			FuncTag,
			L"`IndexByteStride` exceeds provided index data byte size. Stride: `{}`. Data Size: `{}`.",
			descriptor.IndexByteStride, indicesSize
		);

		bool unsupportedIndexStride = descriptor.IndexByteStride != sizeof(uint8_t) &&
			descriptor.IndexByteStride != sizeof(uint16_t);

		m_Logger.LogFatalNLFormattedIf(
			unsupportedIndexStride,
			FuncTag,
			L"`IndexByteStride` is unsupported. Index Byte Stride: `{}`. Supported Index Byte Strides: [2, 4]."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.IndexByteOffset >= indices.size(),
			FuncTag,
			L"`IndexByteOffset` exceeds or reaches provided index data byte size. Offset: `{}`. Data Size: `{}`.",
			descriptor.IndexByteOffset, indicesSize
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.StartIndexLocation >= descriptor.TotalIndices,
			FuncTag,
			L"`StartIndexLocation` is beyond expected valid range of index indices. Start Location: `{}`. Expected range: [0, {}].",
			descriptor.StartIndexLocation,
			descriptor.TotalIndices
		);
		
		/* Widen to prevent the odd case of an overflow. */
		uint64_t widenedTotalIndices = static_cast<uint64_t>(descriptor.TotalIndices);
		uint64_t widenedStartIndexLocation = static_cast<uint64_t>(descriptor.StartIndexLocation);

		m_Logger.LogWarningNLFormattedIf(
			widenedTotalIndices < widenedStartIndexLocation + 3,
			FuncTag,
			L"`StartIndexLocation` is likely malformed: fewer than 3 indices available for drawing "
			L"from this offset. Total: `{}`. Start Location: `{}`.",
			descriptor.TotalIndices,
			descriptor.StartIndexLocation
		);
	}

	void Renderer::EnforceValidDrawDescriptorIndexedInstanced(
		std::span<const std::byte> vertices,
		std::span<const std::byte> indices,
		std::span<const std::byte> instances,
		const DrawDescriptor& descriptor
	) noexcept
	{
		constexpr std::wstring_view FuncTag = L"Renderer [EnforceValidDrawDescriptorIndexedInstanced] |";

		EnforceValidDrawDescriptorIndexed(vertices, indices, descriptor);

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
			L"`TotalInstances` exceeds size of instance buffer. "
			L"TotalInstances: `{}`, Size : `{}`.",
			descriptor.TotalInstances, instancesSize
		);

		m_Logger.LogFatalNLTaggedIf(
			descriptor.InstanceByteStride == 0,
			FuncTag,
			L"`InstanceByteStride` should not be 0."
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.InstanceByteStride > instancesSizeBytes,
			FuncTag,
			L"`InstanceByteStride` exceeds byte size of instance buffer. "
			L"InstanceByteStride: `{}`, Byte Size : `{}`.",
			descriptor.InstanceByteStride, instancesSizeBytes
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.InstanceByteOffset >= instancesSizeBytes,
			FuncTag,
			L"`InstanceByteOffset` exceeds or equals byte size of instance buffer. "
			L"InstanceByteOffset: `{}`, Byte Size: `{}`.",
			descriptor.InstanceByteOffset, instancesSizeBytes
		);

		constexpr UINT MaxRegisterSlot = VertexBuffer::MaxRegisterSlot();

		m_Logger.LogFatalNLFormattedIf(
			!VertexBuffer::IsValidRegister(descriptor.InstanceBufferRegister),
			FuncTag,
			L"`InstanceBufferRegister` must be within [0 - {}]: `{}`. "
			L"",
			MaxRegisterSlot, descriptor.InstanceBufferRegister
		);

		m_Logger.LogFatalNLFormattedIf(
			descriptor.VertexBufferRegister == descriptor.InstanceBufferRegister,
			FuncTag,
			L"`VertexBufferRegister` and `InstanceBufferRegister` clash. "
			L"Two vertex buffers should not have the same register slot: `{}`.",
			descriptor.VertexBufferRegister
		);
	}
#pragma endregion

	void Renderer::InitImGui(const HWND hWnd) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Device.IsCreated(),
			L"Renderer [InitImGui] | Device was not created previously."
		);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(m_Device.GetRaw(), m_Device.ContextRaw());
	}

	void Renderer::ShutdownImGui() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [InitImGui] | Context was not initialized previously."
		);

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void Renderer::CreateRTV() noexcept
	{
		m_Logger.LogFatalNLIf(
			mP_RTV.Get() != nullptr,
			L"Renderer [CreateRTV] | A RTV is still present."
		);

		m_Logger.LogFatalNLIf(
			mP_DSV.Get() != nullptr,
			L"Renderer [CreateRTV] | A DSV is still present."
		);

		// Get the back buffer.
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		HRESULT hResult = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"Renderer [CreateRTV] | Failed to get back buffer : ",
			Win::Utility::TranslateError(hResult)
		);

		// Create the RTV.
		hResult = m_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, mP_RTV.GetAddressOf());

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"Renderer [CreateRTV] | Failed to create render target view : ",
			Win::Utility::TranslateError(hResult)
		);

		D3D11_TEXTURE2D_DESC backBufferDesc;
		pBackBuffer->GetDesc(&backBufferDesc);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencilTexture;
		D3D11_TEXTURE2D_DESC dsTextureDesc = {};
		dsTextureDesc.Width = backBufferDesc.Width;
		dsTextureDesc.Height = backBufferDesc.Height;
		dsTextureDesc.MipLevels = 1u;
		dsTextureDesc.ArraySize = 1u;
		dsTextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		dsTextureDesc.SampleDesc.Count = 1u;
		dsTextureDesc.SampleDesc.Quality = 0;
		dsTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		dsTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hResult = m_Device->CreateTexture2D(&dsTextureDesc, nullptr, pDepthStencilTexture.GetAddressOf());

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"Renderer [CreateRTV] | Failed to create depth stencil texture : ",
			Win::Utility::TranslateError(hResult)
		);

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable = FALSE;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
		hResult = m_Device->CreateDepthStencilState(&depthStencilDesc, pDSState.GetAddressOf());

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"Renderer [CreateRTV] | Failed to create depth stencil state : ",
			Win::Utility::TranslateError(hResult)
		);

		m_Device.ContextRaw()->OMSetDepthStencilState(pDSState.Get(), 1u);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0u;

		hResult = m_Device->CreateDepthStencilView(pDepthStencilTexture.Get(), &dsvDesc, mP_DSV.GetAddressOf());

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"Renderer [CreateRTV] | Failed to create depth stencil view : ",
			Win::Utility::TranslateError(hResult)
		);
	}

	void Renderer::BindRTV() noexcept
	{
		m_Device.ContextRaw()->OMSetRenderTargets(1u, mP_RTV.GetAddressOf(), mP_DSV.Get());
	}

	void Renderer::SetViewport() noexcept
	{
		CD3D11_VIEWPORT viewport(
			0.0f, 
			0.0f, 
			static_cast<FLOAT>(m_State.CurrentClientArea().right),
			static_cast<FLOAT>(m_State.CurrentClientArea().bottom)
		);

		m_Device.ContextRaw()->RSSetViewports(1, &viewport);
	}

	void Renderer::SetTopology() noexcept
	{
		m_Device.ContextRaw()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void Renderer::ResetState() noexcept
	{
		m_Device.ContextRaw()->ClearState();
		m_Device.ContextRaw()->Flush();

		mP_RTV.Reset();
		mP_DSV.Reset();
	}

	void Renderer::OnWindowResize() noexcept
	{
		m_State.FlagResize();

		if (!m_Initialized)
			return;

		ResetState();

		SetViewport();
		SetTopology();

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		HRESULT hResult = m_SwapChain->GetDesc(&swapChainDesc);

		m_Logger.LogFatalNLIf(
			FAILED(hResult),
			L"Renderer [OnWindowResize] | Failed to retrieve swap chain description."
		);

		RECT clientArea = m_State.CurrentClientArea();

		/**
		 * (According to DirectX Graphics Infrastructure (DXGI): Best Practices)
		 * (https://learn.microsoft.com/en-us/windows/win32/direct3darticles/dxgi-best-practices#full-screen-issues)
		 * 
		 * In Direct3D 11, DXGI automatically resizes the front buffer when the window
		 * is resized, but the application should call IDXGISwapChain::ResizeBuffers with
		 * the correct size to resize the back buffer as well.
		 * 
		 * As such, any RTV's and DSV's should be recreated manually by the application when the window resizes.
		 * 
		 * When creating a full-screen swap chain, the Flags member of the DXGI_SWAP_CHAIN_DESC
		 * structure must be set to DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH to override DXGI's 
		 * default behavior. (Usually only for full-screen behavior)
		 */
		hResult = m_SwapChain->ResizeBuffers(
			swapChainDesc.BufferCount,
			static_cast<UINT>(clientArea.right),
			static_cast<UINT>(clientArea.bottom),
			swapChainDesc.BufferDesc.Format,
			swapChainDesc.Flags
		);

		m_Logger.LogFatalNLIf(
			FAILED(hResult),
			L"Renderer [OnWindowResize] | Failed to resize buffers."
		);

		CreateRTV();
		BindRTV();

		/* Update the frame data constant buffer. */
		FrameData frameData = {
			static_cast<float>(clientArea.right),
			static_cast<float>(clientArea.bottom),
			{ 0.0f, 0.0f }
		};

		m_Logger.LogFatalNLIf(
			!m_CBFrameData.IsCreated(),
			L"Renderer [OnWindowResize] | FrameData constant buffer wasn't created previously."
		);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		hResult = m_Device.ContextRaw()->Map(m_CBFrameData.GetRaw(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);

		m_Logger.LogFatalNLIf(
			FAILED(hResult),
			L"Renderer [OnWindowResize] | Failed to update FrameData constant buffer."
		);

		std::memcpy(mappedResource.pData, &frameData, sizeof(FrameData));
		
		m_Device.ContextRaw()->Unmap(m_CBFrameData.GetRaw(), 0u);

		RebindCurrentShaderSet();
	}

	void Renderer::RebindCurrentShaderSet() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"Renderer [RebindCurrentShaderSet] | Context isn't initialized."
		);

		m_State.RebindCurrentShaderSet();
	}
#pragma endregion
}