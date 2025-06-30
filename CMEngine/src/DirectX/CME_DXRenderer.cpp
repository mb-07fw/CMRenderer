#include "Core/CME_PCH.hpp"
#include "DirectX/CME_DXRenderer.hpp"
#include "Windows/CME_WNUtility.hpp"
#include "CMC_Macros.hpp"

namespace CMEngine::DirectXAPI::DX11
{
#pragma region DXRendererState
	DXRendererState::DXRendererState(
		CMCommon::CMLoggerWide& logger,
		DXShaderLibrary& shaderLibrary,
		DXDevice& device,
		const CMWindowData& currentWindowData
	) noexcept
		: m_ShaderLibrary(shaderLibrary), 
		  m_Device(device),
		  m_CurrentWindowData(currentWindowData),
		  m_Camera(),
		  m_Logger(logger)
	{
	}

	void DXRendererState::SetCurrentShaderSet(DXShaderSetType shaderType) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Device.IsCreated(),
			L"DXRendererState [SetCurrentShaderSet] | Device is not created."
		);

		m_Logger.LogFatalNLIf(
			!m_ShaderLibrary.IsInitialized(),
			L"DXRendererState [SetCurrentShaderSet] | Shader library is not initialized."
		);

		std::shared_ptr<IDXShaderSet> pCurrentShaderSet = nullptr;
		if (!mP_CurrentShaderSet.expired())
		{
			pCurrentShaderSet = mP_CurrentShaderSet.lock();

			if (m_Logger.LogWarningNLIf(pCurrentShaderSet->Type == shaderType, L"DXRendererState [SetCurrentShaderSet] | DXShaderSetType is already set."))
				return;
		}

		mP_CurrentShaderSet = m_ShaderLibrary.GetSetOfType(shaderType);

		m_Logger.LogFatalNLIf(
			mP_CurrentShaderSet.expired(),
			L"DXRendererState [SetCurrentShaderSet] | Retrieved shader set pointer is expired."
		);

		if (pCurrentShaderSet == nullptr)
			pCurrentShaderSet = mP_CurrentShaderSet.lock();

		m_Logger.LogFatalNLIf(
			!pCurrentShaderSet->IsCreated,
			L"DXRendererState [SetCurrentShaderSet] | Retrieved shader set wasn't created previously."
		);

		m_Logger.LogFatalNLIf(
			pCurrentShaderSet->pVertexShader.Get() == nullptr,
			L"DXRendererState [SetCurrentShaderSet] | Current shader set's vertex shader was nullptr."
		);

		m_Logger.LogFatalNLIf(
			pCurrentShaderSet->pPixelShader.Get() == nullptr,
			L"DXRendererState [SetCurrentShaderSet] | Current shader set's pixel shader was nullptr."
		);

		m_Logger.LogFatalNLIf(
			pCurrentShaderSet->pInputLayout.Get() == nullptr,
			L"DXRendererState [SetCurrentShaderSet] | Current shader set's input layout was nullptr."
		);

		m_Device.ContextRaw()->VSSetShader(pCurrentShaderSet->pVertexShader.Get(), nullptr, 0);
		m_Device.ContextRaw()->PSSetShader(pCurrentShaderSet->pPixelShader.Get(), nullptr, 0);
		m_Device.ContextRaw()->IASetInputLayout(pCurrentShaderSet->pInputLayout.Get());
	}

	void DXRendererState::SetCurrentModelMatrix(const DirectX::XMMATRIX& modelMatrix) noexcept
	{
		m_CurrentModelMatrix = modelMatrix;
		m_ModelMatrixSet = true;
	}

	void DXRendererState::SetCurrentCameraData(const CMCameraData& cameraData) noexcept
	{
		m_CameraData = cameraData;
		m_CameraUpdated = true;
	}

	void DXRendererState::SetCurrentCameraTransform(const CMCommon::CMRigidTransform& rigidTransform) noexcept
	{
		m_CameraData.RigidTransform = rigidTransform;
		m_CameraTransformUpdated = true;
		m_CameraUpdated = true;
	}

	void DXRendererState::RebindCurrentShaderSet() noexcept
	{
		m_Logger.LogFatalNLIf(
			mP_CurrentShaderSet.expired(),
			L"DXRendererState [RebindCurrentShaderSet] | No shader set is currently set."
		);

		DXShaderSetType currentSet = mP_CurrentShaderSet.lock()->Type;

		mP_CurrentShaderSet.reset();

		SetCurrentShaderSet(currentSet);
	}

	void DXRendererState::FlagResize() noexcept
	{
		if (CurrentAspectRatio() == m_Camera.AspectRatio())
			return;
		
		m_WindowResized = true;
	}
	
	void DXRendererState::UpdateCamera() noexcept
	{
		if (!m_CameraUpdated && !m_WindowResized)
		{
			m_Logger.LogWarningNL(
				L"DXRendererState [UpdateCamera] | Attempted to update camera even "
				L"though camera data and window resolution haven't changed."
			);

			return;
		}

		/* Camera updated and window resized, View, Projection and ViewProjection matrices need to be recalculated. */
		if (m_CameraUpdated && m_WindowResized)
			m_Camera.SetAll(m_CameraData, CurrentAspectRatio());
		/* Camera transform updated, only View and ViewProjection matrices need to be recalculated. */
		else if (m_CameraTransformUpdated)
			m_Camera.SetTransform(m_CameraData.RigidTransform);
		/* Window resolution updated, only Projection and ViewProjection matrices need to be recalculated. */
		else if (m_WindowResized)
			m_Camera.SetAspectRatio(CurrentAspectRatio());

		m_CameraUpdated = false;
		m_CameraTransformUpdated = false;
		m_WindowResized = false;
	}

	[[nodiscard]] DXShaderSetType DXRendererState::CurrentShaderSet() const noexcept
	{
		if (mP_CurrentShaderSet.expired())
			return DXShaderSetType::INVALID;

		std::shared_ptr<IDXShaderSet> pCurrentShaderSet = mP_CurrentShaderSet.lock();

		return pCurrentShaderSet.get() == nullptr ? DXShaderSetType::INVALID :
			pCurrentShaderSet->Type;
	}

	[[nodiscard]] float DXRendererState::CurrentAspectRatio() const noexcept
	{
		return static_cast<float>(m_CurrentWindowData.ClientArea.right) /
			m_CurrentWindowData.ClientArea.bottom;
	}
#pragma endregion

#pragma region DXRenderer

#define COMMA ,

	DXRenderer::DXRenderer(CMCommon::CMLoggerWide& logger, const CMWindowData& currentWindowData) noexcept
		: m_Logger(logger),
		  m_ShaderLibrary(logger),
		  m_State(logger, m_ShaderLibrary, m_Device, currentWindowData),
		  m_Device(logger),
		  m_Factory(logger),
		  m_SwapChain(logger),
		  //m_Writer(logger),
		  m_CBFrameData(CMShaderConstants::S_FRAME_DATA_REGISTER_SLOT, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE) CM_IF_NDEBUG_REPLACE(COMMA)
		  CM_IF_NDEBUG_REPLACE(m_InfoQueue(logger))
	{
		CM_IF_DEBUG(
			mP_DebugInterfaceModule = LoadLibrary(L"Dxgidebug.dll");
		
			m_Logger.LogFatalNLIf(mP_DebugInterfaceModule == nullptr, L"DXRenderer [()] | Failed to load Dxgidebug.dll");

			// Here so Intelisense doesn't yell at me for de-referencing a nullptr, even though fatal logs terminate the program.
			if (mP_DebugInterfaceModule == nullptr)
				return;

			typedef HRESULT(WINAPI* DXGIGetDebugInterfaceFunc)(const IID&, void**);

			// Retrieve the address of DXGIGetDebugInterface function.
			DXGIGetDebugInterfaceFunc pDXGIGetDebugInterface =
				(DXGIGetDebugInterfaceFunc)GetProcAddress(mP_DebugInterfaceModule, "DXGIGetDebugInterface");

			m_Logger.LogFatalNLIf(pDXGIGetDebugInterface == nullptr, L"DXRenderer [()] | Failed to get function address for the DXGIGetDebugInterface.");

			// Here so Intelisense doesn't yell at me for de-referencing a nullptr, even though fatal logs terminate the program.
			if (pDXGIGetDebugInterface == nullptr)
				return;

			HRESULT hResult = pDXGIGetDebugInterface(IID_PPV_ARGS(&mP_DebugInterface));

			m_Logger.LogFatalNLIf(
				FAILED(hResult),
				L"DXRenderer [()] | Failed to retrieve a DXGI debug interface."
			);
		);

		m_Logger.LogInfoNL(L"DXRenderer [()] | Constructed.");
	}

	DXRenderer::~DXRenderer() noexcept
	{
		if (m_Initialized)
			Shutdown();

		CM_IF_DEBUG(FreeLibrary(mP_DebugInterfaceModule));

		m_Logger.LogInfoNL(L"DXRenderer [~()] | Destroyed.");
	}

	void DXRenderer::Init(const HWND hWnd) noexcept
	{
		m_Logger.LogWarningNLIf(
			m_Initialized,
			L"DXRenderer [Init] | Initializion has been attempted after CMRenderContext has already been initialized."
		);

		m_Device.Create();
		m_Factory.Create(m_Device);
		m_SwapChain.Create(hWnd, m_State.CurrentClientArea(), m_Factory, m_Device);
		//m_Writer.Create(m_SwapChain);

		CM_IF_DEBUG(
			m_InfoQueue.Create(m_Device);

			m_Logger.LogFatalNLIf(
				!m_InfoQueue.IsCreated(),
				L"DXRenderer [Init] | Failed to initialize info queue."
			);
		);

		InitImGui(hWnd);

		m_ShaderLibrary.Init(m_Device);

		CMFrameData frameData = {
			static_cast<float>(m_State.CurrentClientArea().right),
			static_cast<float>(m_State.CurrentClientArea().bottom),
			{ 0.0f, 0.0f }
		};

		m_CBFrameData.SetData(std::span(&frameData, 1u));

		m_Logger.LogFatalNLIf(
			FAILED(m_CBFrameData.Create(m_Device)),
			L"DXRenderer [Init] | Failed to create CMFrameData constant buffer."
		);

		m_CBFrameData.BindPS(m_Device);

		m_Factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		CreateRTV();
		SetViewport();
		SetTopology();

		m_Logger.LogInfoNL(L"DXRenderer [Init] | Initialized.");

		m_Initialized = true;
		m_Shutdown = false;
	}

	void DXRenderer::Shutdown() noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Shutdown, 
			L"DXRenderer [Shutdown] | Shutdown has been attempted after shutdown has already occured previously."
		);

		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [Shutdown] | Shutdown has been attempted before initialization."
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

	void DXRenderer::SetShaderSet(DXShaderSetType setType) noexcept
	{
		m_Logger.LogFatalNLIf(
			setType == DXShaderSetType::INVALID,
			L"DXRenderer [SetShaderSet] | The provided DXShaderSetType should not be DXShaderSetType::INVALID."
		);

		if (m_State.CurrentShaderSet() == setType)
		{
			m_Logger.LogWarningNL(L"DXRenderer [SetShaderSet] | The provided DXShaderSetType is already set.");
			return;
		}

		m_State.SetCurrentShaderSet(setType);
	}

	void DXRenderer::SetModelMatrix(const DirectX::XMMATRIX& modelMatrix) noexcept
	{
		m_State.SetCurrentModelMatrix(modelMatrix);
	}

	void DXRenderer::SetCamera(const CMCameraData& cameraData) noexcept
	{
		m_State.SetCurrentCameraData(cameraData);
	}

	void DXRenderer::SetCameraTransform(const CMCommon::CMRigidTransform& rigidTransform) noexcept
	{
		m_State.SetCurrentCameraTransform(rigidTransform);
	}

	void DXRenderer::Clear(CMCommon::NormColor normColor) noexcept
	{
		m_Device.ContextRaw()->ClearRenderTargetView(mP_RTV.Get(), normColor.rgba);
		m_Device.ContextRaw()->ClearDepthStencilView(mP_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

		ImGuiNewFrame();
	}

	void DXRenderer::Present() noexcept
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
				L"DXRenderer [Present] | Device error : ",
				WindowsAPI::Utility::TranslateError(hResult)
			);
		}
		else if (FAILED(hResult))
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
					);

			m_Logger.LogFatalNLAppend(
				L"DXRenderer [Present] | Present error : ",
				WindowsAPI::Utility::TranslateError(hResult)
			);
		}
	}

#pragma region ImGui Wrappers
	void DXRenderer::ImGuiNewFrame() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiNewFrame] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void DXRenderer::ImGuiBegin(std::string_view windowTitle, bool* pIsOpen, ImGuiWindowFlags windowFlags) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiBegin] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::Begin(windowTitle.data(), pIsOpen, windowFlags);
	}

	void DXRenderer::ImGuiEnd() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiEnd] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::End();
	}

	bool DXRenderer::ImGuiBeginChild(
		std::string_view stringID,
		ImVec2 size,
		ImGuiChildFlags childFlags,
		ImGuiWindowFlags windowFlags
	) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiBeginChild] Attempted to perform ImGui operation before context was initialized."
		);
	
		return ImGui::BeginChild(stringID.data(), size, childFlags, windowFlags);
	}

	bool DXRenderer::ImGuiBeginChild(
		ImGuiID id,
		ImVec2 size,
		ImGuiChildFlags childFlags,
		ImGuiWindowFlags windowFlags
	) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiBeginChild] Attempted to perform ImGui operation before context was initialized."
		);

		return ImGui::BeginChild(id, size, childFlags, windowFlags);
	}

	void DXRenderer::ImGuiSlider(std::string_view label, float* pValue, float valueMin, float valueMax) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiSlider] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::SliderFloat(label.data(), pValue, valueMin, valueMax);
	}

	void DXRenderer::ImGuiSliderAngle(std::string_view label, float* pRadians, float angleMin, float angleMax) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiSliderAngle] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::SliderAngle(label.data(), pRadians, angleMin, angleMax);
	}

	void DXRenderer::ImGuiShowDemoWindow() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiShowDemoWindow] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::ShowDemoWindow();
	}

	[[nodiscard]] bool DXRenderer::ImGuiCollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiCollapsingHeader] Attempted to perform ImGui operation before context was initialized."
		);

		return ImGui::CollapsingHeader(label.data(), flags);
	}

	[[nodiscard]] bool DXRenderer::ImGuiButton(std::string_view label, ImVec2 size) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiButton] Attempted to perform ImGui operation before context was initialized."
		);

		return ImGui::Button(label.data(), size);
	}

	void DXRenderer::ImGuiEndFrame() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [ImGuiEndFrame] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void DXRenderer::ImGuiEndChild() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized, 
			L"DXRenderer [ImGuiEndChild] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::EndChild();
	}
#pragma endregion

	void DXRenderer::ReportLiveObjects() noexcept
	{
		CM_IF_DEBUG(
			HRESULT hResult = mP_DebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

			m_Logger.LogFatalNLIf(
				FAILED(hResult),
				L"DXRenderer [DrawIndexed] | Failed to report live objects."
			)
		);
	}

	[[nodiscard]] CMCommon::CMRect DXRenderer::CurrentRenderArea() const noexcept
	{
		RECT clientArea = m_State.CurrentClientArea();

		return CMCommon::CMRect(
			static_cast<float>(clientArea.left),
			static_cast<float>(clientArea.right),
			static_cast<float>(clientArea.top),
			static_cast<float>(clientArea.bottom)
		);
	}

	[[nodiscard]] bool DXRenderer::IsFullscreen() noexcept
	{
		BOOL isFullscreen = 0;

		HRESULT hResult = m_SwapChain->GetFullscreenState(&isFullscreen, nullptr);

		m_Logger.LogFatalNLIf(
			FAILED(hResult),
			L"DXRenderer [IsFullscreen] | Failed to get fullscreen state."
		);

		return static_cast<bool>(isFullscreen);
	}

	void DXRenderer::InitImGui(const HWND hWnd) noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Device.IsCreated(),
			L"DXRenderer [InitImGui] | Device was not created previously."
		);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(m_Device.DeviceRaw(), m_Device.ContextRaw());
	}

	void DXRenderer::ShutdownImGui() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [InitImGui] | Context was not initialized previously."
		);

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void DXRenderer::CreateRTV() noexcept
	{
		m_Logger.LogFatalNLIf(
			mP_RTV.Get() != nullptr,
			L"DXRenderer [CreateRTV] | A RTV is still present."
		);

		m_Logger.LogFatalNLIf(
			mP_DSV.Get() != nullptr,
			L"DXRenderer [CreateRTV] | A DSV is still present."
		);

		// Get the back buffer.
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		HRESULT hResult = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"DXRenderer [CreateRTV] | Failed to get back buffer : ",
			WindowsAPI::Utility::TranslateError(hResult)
		);

		// Create the RTV.
		hResult = m_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, mP_RTV.GetAddressOf());

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"DXRenderer [CreateRTV] | Failed to create render target view : ",
			WindowsAPI::Utility::TranslateError(hResult)
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
			L"DXRenderer [CreateRTV] | Failed to create depth stencil texture : ",
			WindowsAPI::Utility::TranslateError(hResult)
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
			L"DXRenderer [CreateRTV] | Failed to create depth stencil state : ",
			WindowsAPI::Utility::TranslateError(hResult)
		);

		m_Device.ContextRaw()->OMSetDepthStencilState(pDSState.Get(), 1u);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0u;

		hResult = m_Device->CreateDepthStencilView(pDepthStencilTexture.Get(), &dsvDesc, mP_DSV.GetAddressOf());

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"DXRenderer [CreateRTV] | Failed to create depth stencil view : ",
			WindowsAPI::Utility::TranslateError(hResult)
		);
	}

	void DXRenderer::BindRTV() noexcept
	{
		m_Device.ContextRaw()->OMSetRenderTargets(1u, mP_RTV.GetAddressOf(), mP_DSV.Get());
	}

	void DXRenderer::SetViewport() noexcept
	{
		CD3D11_VIEWPORT viewport(
			0.0f, 
			0.0f, 
			static_cast<FLOAT>(m_State.CurrentClientArea().right),
			static_cast<FLOAT>(m_State.CurrentClientArea().bottom)
		);

		m_Device.ContextRaw()->RSSetViewports(1, &viewport);
	}

	void DXRenderer::SetTopology() noexcept
	{
		m_Device.ContextRaw()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void DXRenderer::ResetState() noexcept
	{
		m_Device.ContextRaw()->ClearState();
		m_Device.ContextRaw()->Flush();

		mP_RTV.Reset();
		mP_DSV.Reset();
	}

	void DXRenderer::OnWindowResize() noexcept
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
			L"DXRenderer [OnWindowResize] | Failed to retrieve swap chain description."
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
			L"DXRenderer [OnWindowResize] | Failed to resize buffers."
		);

		CreateRTV();
		BindRTV();

		/* Update the frame data constant buffer. */
		CMFrameData frameData = {
			static_cast<float>(clientArea.right),
			static_cast<float>(clientArea.bottom),
			{ 0.0f, 0.0f }
		};

		m_Logger.LogFatalNLIf(
			!m_CBFrameData.IsCreated(),
			L"DXRenderer [OnWindowResize] | CMFrameData constant buffer wasn't created previously."
		);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		hResult = m_Device.ContextRaw()->Map(m_CBFrameData.Buffer(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);

		m_Logger.LogFatalNLIf(
			FAILED(hResult),
			L"DXRenderer [OnWindowResize] | Failed to update FrameData constant buffer."
		);

		std::memcpy(mappedResource.pData, &frameData, sizeof(CMFrameData));
		
		m_Device.ContextRaw()->Unmap(m_CBFrameData.Buffer(), 0u);

		RebindCurrentShaderSet();
	}

	void DXRenderer::RebindCurrentShaderSet() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Initialized,
			L"DXRenderer [RebindCurrentShaderSet] | Context isn't initialized."
		);

		m_State.RebindCurrentShaderSet();
	}
#pragma endregion
}