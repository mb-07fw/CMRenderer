#include "CMR_PCH.hpp"
#include "CMR_DXContext.hpp"
#include "CMC_WindowsUtility.hpp"
#include "CMC_Macros.hpp"

namespace CMRenderer::CMDirectX
{
#pragma region DXContextState
	DXContextState::DXContextState(
		CMCommon::CMLoggerWide& cmLoggerRef,
		DXShaderLibrary& shaderLibraryRef,
		Components::DXDevice& deviceRef,
		const CMWindowData& currentWindowDataRef
	) noexcept
		: m_ShaderLibraryRef(shaderLibraryRef), 
		  m_DeviceRef(deviceRef),
		  m_CurrentWindowDataRef(currentWindowDataRef),
		  m_Camera(),
		  m_CMLoggerRef(cmLoggerRef)
	{
	}

	void DXContextState::SetCurrentShaderSet(DXShaderSetType shaderType) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_DeviceRef.IsCreated(), L"DXContextState [SetCurrentShaderSet] | Device is not created.");
		m_CMLoggerRef.LogFatalNLIf(!m_ShaderLibraryRef.IsInitialized(), L"DXContextState [SetCurrentShaderSet] | Shader library is not initialized.");

		std::shared_ptr<IDXShaderSet> pCurrentShaderSet = nullptr;
		if (!mP_CurrentShaderSet.expired())
		{
			pCurrentShaderSet = mP_CurrentShaderSet.lock();

			if (m_CMLoggerRef.LogWarningNLIf(pCurrentShaderSet->Type == shaderType, L"DXContextState [SetCurrentShaderSet] | DXShaderSetType is already set."))
				return;
		}

		mP_CurrentShaderSet = m_ShaderLibraryRef.GetSetOfType(shaderType);

		m_CMLoggerRef.LogFatalNLIf(mP_CurrentShaderSet.expired(), L"DXContextState [SetCurrentShaderSet] | Retrieved shader set pointer is expired.");

		if (pCurrentShaderSet == nullptr)
			pCurrentShaderSet = mP_CurrentShaderSet.lock();

		m_CMLoggerRef.LogFatalNLIf(!pCurrentShaderSet->IsCreated, L"DXContextState [SetCurrentShaderSet] | Retrieved shader set wasn't created previously.");

		m_CMLoggerRef.LogFatalNLIf(pCurrentShaderSet->pVertexShader.Get() == nullptr, L"DXContextState [SetCurrentShaderSet] | Vertex Shader was nullptr.");
		m_CMLoggerRef.LogFatalNLIf(pCurrentShaderSet->pPixelShader.Get() == nullptr, L"DXContextState [SetCurrentShaderSet] | Pixel Shader was nullptr.");
		m_CMLoggerRef.LogFatalNLIf(pCurrentShaderSet->pInputLayout.Get() == nullptr, L"DXContextState [SetCurrentShaderSet] | Input Layout was nullptr.");

		m_DeviceRef.ContextRaw()->VSSetShader(pCurrentShaderSet->pVertexShader.Get(), nullptr, 0);
		m_DeviceRef.ContextRaw()->PSSetShader(pCurrentShaderSet->pPixelShader.Get(), nullptr, 0);
		m_DeviceRef.ContextRaw()->IASetInputLayout(pCurrentShaderSet->pInputLayout.Get());
	}

	void DXContextState::SetCurrentModelMatrix(const DirectX::XMMATRIX& modelMatrixRef) noexcept
	{
		m_CurrentModelMatrix = modelMatrixRef;
		m_ModelMatrixSet = true;
	}

	void DXContextState::SetCurrentCameraData(const CMCameraData& cameraDataRef) noexcept
	{
		m_CameraData = cameraDataRef;
		m_CameraUpdated = true;
	}

	void DXContextState::SetCurrentCameraTransform(const CMCommon::CMRigidTransform& rigidTransformRef) noexcept
	{
		m_CameraData.RigidTransform = rigidTransformRef;
		m_CameraTransformUpdated = true;
		m_CameraUpdated = true;
	}

	void DXContextState::RebindCurrentShaderSet() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(mP_CurrentShaderSet.expired(), L"DXContextState [RebindCurrentShaderSet] | No shader set is currently set.");

		DXShaderSetType currentSet = mP_CurrentShaderSet.lock()->Type;

		mP_CurrentShaderSet.reset();

		SetCurrentShaderSet(currentSet);
	}

	void DXContextState::UpdateResolution() noexcept
	{
		if (CurrentAspectRatio() == m_Camera.AspectRatio())
			return;
		
		m_WindowResized = true;
	}
	
	void DXContextState::UpdateCamera() noexcept
	{
		if (!m_CameraUpdated && !m_WindowResized)
		{
			m_CMLoggerRef.LogWarningNL(
				L"DXContextState [UpdateCamera] | Attempted to update camera even though camera data "
				L" and window resolution haven't changed."
			);
			return;
		}

		/* Camera updated and window resized, all V, P and VP matrices need to be recalculated. */
		if (m_CameraUpdated && m_WindowResized)
			m_Camera.SetAll(m_CameraData, CurrentAspectRatio());
		/* Camera transform updated, only V and VP matrices need to be recalculated. */
		else if (m_CameraTransformUpdated)
			m_Camera.SetTransform(m_CameraData.RigidTransform);
		/* Window resolution updated, only P and VP matrices need to be recalculated. */
		else if (m_WindowResized)
			m_Camera.SetAspectRatio(CurrentAspectRatio());

		m_CameraUpdated = false;
		m_CameraTransformUpdated = false;
		m_WindowResized = false;
	}

	[[nodiscard]] DXShaderSetType DXContextState::CurrentShaderSet() const noexcept
	{
		if (mP_CurrentShaderSet.expired())
			return DXShaderSetType::INVALID;

		std::shared_ptr<IDXShaderSet> pCurrentShaderSet = mP_CurrentShaderSet.lock();

		return pCurrentShaderSet.get() == nullptr ? DXShaderSetType::INVALID :
			pCurrentShaderSet->Type;
	}

	[[nodiscard]] float DXContextState::CurrentAspectRatio() const noexcept
	{
		return static_cast<float>(m_CurrentWindowDataRef.ClientArea.right) /
			m_CurrentWindowDataRef.ClientArea.bottom;
	}
#pragma endregion

#pragma region DXContext

#define COMMA ,

	DXContext::DXContext(CMCommon::CMLoggerWide& cmLoggerRef, const CMWindowData& currentWindowDataRef) noexcept
		: m_CMLoggerRef(cmLoggerRef),
		  m_ShaderLibrary(cmLoggerRef),
		  m_State(cmLoggerRef, m_ShaderLibrary, m_Device, currentWindowDataRef),
		  m_Device(cmLoggerRef),
		  m_Factory(cmLoggerRef), 
		  m_SwapChain(cmLoggerRef),
		  m_Writer(cmLoggerRef),
		  m_CBFrameData(CMShaderConstants::S_FRAME_DATA_REGISTER_SLOT, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE) CM_IF_NDEBUG_REPLACE(COMMA)
		  CM_IF_NDEBUG_REPLACE(m_InfoQueue(cmLoggerRef))
	{
		CM_IF_DEBUG(
			m_DebugInterfaceModule = LoadLibrary(L"Dxgidebug.dll");
		
			m_CMLoggerRef.LogFatalNLIf(m_DebugInterfaceModule == nullptr, L"DXContext [()] | Failed to load Dxgidebug.dll");

			// Here so Intelisense doesn't yell at me for de-referencing a nullptr, even though fatal logs terminate the program.
			if (m_DebugInterfaceModule == nullptr)
				return;

			typedef HRESULT(WINAPI* DXGIGetDebugInterfaceFunc)(const IID&, void**);

			// Retrieve the address of DXGIGetDebugInterface function.
			DXGIGetDebugInterfaceFunc pDXGIGetDebugInterface =
				(DXGIGetDebugInterfaceFunc)GetProcAddress(m_DebugInterfaceModule, "DXGIGetDebugInterface");

			m_CMLoggerRef.LogFatalNLIf(pDXGIGetDebugInterface == nullptr, L"DXContext [()] | Failed to get function address for the DXGIGetDebugInterface.");

			// Here so Intelisense doesn't yell at me for de-referencing a nullptr, even though fatal logs terminate the program.
			if (pDXGIGetDebugInterface == nullptr)
				return;

			HRESULT hResult = pDXGIGetDebugInterface(IID_PPV_ARGS(&mP_DebugInterface));

			m_CMLoggerRef.LogFatalNLIf(hResult != S_OK, L"DXContext [()] | Failed to retrieve a DXGI debug interface.");
		);

		m_CMLoggerRef.LogInfoNL(L"DXContext [()] | Constructed.");
	}

	DXContext::~DXContext() noexcept
	{
		if (m_Initialized)
			Shutdown();

		CM_IF_DEBUG(FreeLibrary(m_DebugInterfaceModule));

		m_CMLoggerRef.LogInfoNL(L"DXContext [~()] | Destroyed.");
	}

	void DXContext::Init(const HWND hWnd) noexcept
	{
		m_CMLoggerRef.LogWarningNLIf(m_Initialized, L"DXContext [Init] | Initializion has been attempted after CMRenderContext has already been initialized.");

		m_Device.Create();
		m_Factory.Create(m_Device);
		m_SwapChain.Create(hWnd, m_State.CurrentClientArea(), m_Factory, m_Device);
		//m_Writer.Create(m_SwapChain);

		CM_IF_DEBUG(
			m_InfoQueue.Create(m_Device);
			m_CMLoggerRef.LogFatalNLIf(!m_InfoQueue.IsCreated(), L"DXContext [Init] | Failed to initialize info queue.");
		);

		InitImGui(hWnd);

		m_ShaderLibrary.Init(m_Device);

		/* Rasterizer testing...
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.FrontCounterClockwise = (BOOL)true;

		HRESULT hResult = m_Device->CreateRasterizerState(&rasterizerDesc, mP_RasterizerState.GetAddressOf());

		m_CMLoggerRef.LogFatalNL(hResult != S_OK, L"DXContext [Init] | Failed to create rasterizer state.");

		m_Device.ContextRaw()->RSSetState(mP_RasterizerState.Get());*/

		CMFrameData frameData = {
			static_cast<float>(m_State.CurrentClientArea().right),
			static_cast<float>(m_State.CurrentClientArea().bottom),
			{ 0.0f, 0.0f }
		};

		m_CBFrameData.SetData(std::span(&frameData, 1u));

		m_CMLoggerRef.LogFatalNLIf(
			FAILED(m_CBFrameData.Create(m_Device)),
			L"DXContext [Init] | Failed to create CMFrameData constant buffer."
		);

		m_CBFrameData.BindPS(m_Device);

		m_Factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		CreateRTV();
		SetViewport();
		SetTopology();

		m_Initialized = true;
		m_Shutdown = false;

		m_CMLoggerRef.LogInfoNL(L"DXContext [Init] | Initialized.");
	}

	void DXContext::Shutdown() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Shutdown, L"DXContext [Shutdown] | Shutdown has been attempted after shutdown has already occured previously.");
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"DXContext [Shutdown] | Shutdown has been attempted before initialization.");

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

	void DXContext::SetShaderSet(DXShaderSetType setType) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(setType == DXShaderSetType::INVALID, L"DXContext [SetShaderSet] | The provided DXShaderSetType should not be DXShaderSetType::INVALID.");

		if (m_State.CurrentShaderSet() == setType)
		{
			m_CMLoggerRef.LogWarningNL(L"DXContext [SetShaderSet] | The provided DXShaderSetType is already set.");
			return;
		}

		m_State.SetCurrentShaderSet(setType);
	}

	void DXContext::SetModelMatrix(const DirectX::XMMATRIX& modelMatrixRef) noexcept
	{
		m_State.SetCurrentModelMatrix(modelMatrixRef);
	}

	void DXContext::SetCamera(const CMCameraData& cameraDataRef) noexcept
	{
		m_State.SetCurrentCameraData(cameraDataRef);
	}

	void DXContext::SetCameraTransform(const CMCommon::CMRigidTransform& rigidTransformRef) noexcept
	{
		m_State.SetCurrentCameraTransform(rigidTransformRef);
	}

	void DXContext::Clear(CMCommon::NormColor normColor) noexcept
	{
		m_Device.ContextRaw()->ClearRenderTargetView(mP_RTV.Get(), normColor.rgba);
		m_Device.ContextRaw()->ClearDepthStencilView(mP_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

		ImGuiNewFrame();
	}

	void DXContext::Present() noexcept
	{
		ImGuiEndFrame();

		HRESULT hResult = m_SwapChain->Present(1, 0);

		if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
					);

			m_CMLoggerRef.LogFatalNLAppend(
				L"DXContext [Present] | Device error : ",
				WindowsUtility::TranslateDWORDError(hResult)
			);
		}
		else if (FAILED(hResult))
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
					);

			m_CMLoggerRef.LogFatalNLAppend(
				L"DXContext [Present] | Present error : ",
				WindowsUtility::TranslateDWORDError(hResult)
			);
		}
	}

#pragma region ImGui Wrappers
	void DXContext::ImGuiNewFrame() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiNewFrame] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void DXContext::ImGuiBegin(std::string_view windowTitle, bool* pIsOpen, ImGuiWindowFlags windowFlags) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiBegin] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::Begin(windowTitle.data(), pIsOpen, windowFlags);
	}

	void DXContext::ImGuiEnd() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiEnd] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::End();
	}

	bool DXContext::ImGuiBeginChild(
		std::string_view stringID,
		ImVec2 size,
		ImGuiChildFlags childFlags,
		ImGuiWindowFlags windowFlags
	) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiBeginChild] Attempted to perform ImGui operation before context was initialized."
		);
	
		return ImGui::BeginChild(stringID.data(), size, childFlags, windowFlags);
	}

	bool DXContext::ImGuiBeginChild(
		ImGuiID id,
		ImVec2 size,
		ImGuiChildFlags childFlags,
		ImGuiWindowFlags windowFlags
	) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiBeginChild] Attempted to perform ImGui operation before context was initialized."
		);

		return ImGui::BeginChild(id, size, childFlags, windowFlags);
	}

	void DXContext::ImGuiSlider(std::string_view label, float* pValue, float valueMin, float valueMax) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiSlider] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::SliderFloat(label.data(), pValue, valueMin, valueMax);
	}

	void DXContext::ImGuiSliderAngle(std::string_view label, float* pRadians, float angleMin, float angleMax) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiSliderAngle] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::SliderAngle(label.data(), pRadians, angleMin, angleMax);
	}

	void DXContext::ImGuiShowDemoWindow() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiShowDemoWindow] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::ShowDemoWindow();
	}

	[[nodiscard]] bool DXContext::ImGuiCollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiCollapsingHeader] Attempted to perform ImGui operation before context was initialized."
		);

		return ImGui::CollapsingHeader(label.data(), flags);
	}

	[[nodiscard]] bool DXContext::ImGuiButton(std::string_view label, ImVec2 size) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiButton] Attempted to perform ImGui operation before context was initialized."
		);

		return ImGui::Button(label.data(), size);
	}

	void DXContext::ImGuiEndFrame() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized,
			L"DXContext [ImGuiEndFrame] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void DXContext::ImGuiEndChild() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(
			!m_Initialized, 
			L"DXContext [ImGuiEndChild] Attempted to perform ImGui operation before context was initialized."
		);

		ImGui::EndChild();
	}

	void DXContext::ReportLiveObjects() noexcept
	{
		CM_IF_DEBUG(
			HRESULT hResult = mP_DebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

			m_CMLoggerRef.LogFatalNLIf(FAILED(hResult), L"DXContext [DrawIndexed] | Failed to report live objects.");
		)
	}

	[[nodiscard]] bool DXContext::IsFullscreen() noexcept
	{
		BOOL isFullscreen = 0;

		HRESULT hResult = m_SwapChain->GetFullscreenState(&isFullscreen, nullptr);

		m_CMLoggerRef.LogFatalNLIf(FAILED(hResult), L"DXContext [IsFullscreen] | Failed to get fullscreen state.");

		return static_cast<bool>(isFullscreen);
	}
#pragma endregion

#pragma region Test Drawing
	/*void DXContext::TestDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY, float offsetZ) noexcept
	{
		BindRTV();

		struct Vertex3D {
			struct Pos {
				float x, y, z;
			} pos;
		};

		struct CBTransform {
			DirectX::XMMATRIX transform;
		};

		struct Color {
			float r, g, b, a;
		};

		struct CBColors {
			Color colors[6];
		};

		std::array<Vertex3D, 8> vertices = {{
			{ Vertex3D::Pos{ -1.0f, -1.0f, -1.0f } },
			{ Vertex3D::Pos{  1.0f, -1.0f, -1.0f } },
			{ Vertex3D::Pos{ -1.0f,  1.0f, -1.0f } },
			{ Vertex3D::Pos{  1.0f,  1.0f, -1.0f } },
												  
			{ Vertex3D::Pos{ -1.0f, -1.0f,  1.0f } },
			{ Vertex3D::Pos{  1.0f, -1.0f,  1.0f } },
			{ Vertex3D::Pos{ -1.0f,  1.0f,  1.0f } },
			{ Vertex3D::Pos{  1.0f,  1.0f,  1.0f } },
		}};

		std::array<uint16_t, 36> indices = {
			0, 2, 1,
			2, 3, 1,
			
			1, 3, 5,
			3, 7, 5,

			2, 6, 3,
			3, 6, 7,

			4, 5, 7,
			4, 7, 6,

			0, 4, 2,
			2, 4, 6,
			
			0, 1, 4,
			1, 5, 4
		};

		

		DXShaderSet& shaderSet = m_ShaderLibrary.GetSetOfType(DXImplementedShaderType::DEFAULT3D);

		if (!shaderSet.IsCreated())
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | Shader set wasn't created previously.");

		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;

		HRESULT hResult = m_Device->CreateInputLayout(
			shaderSet.Desc().Data(),
			(UINT)shaderSet.Desc().Size(),
			shaderSet.VertexData().pBytecode->GetBufferPointer(),
			shaderSet.VertexData().pBytecode->GetBufferSize(),
			&pInputLayout
		);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [TestDraw] | An error occured when creating the input layout.\n");
		}

		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pCBTransform;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pCBColors;

		UINT stride = sizeof(DXPos2DInterColorInput);
		UINT offset = 0;

		CD3D11_BUFFER_DESC vDesc((UINT)vertices.size() * sizeof(DXPos2DInterColorInput), D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA vData = {};
		vData.pSysMem = vertices.data();

		hResult = m_Device->CreateBuffer(&vDesc, &vData, pVertexBuffer.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | An error occured when creating the vertex buffer.");
		}

		CD3D11_BUFFER_DESC iDesc(sizeof(uint16_t) * (UINT)indices.size(), D3D11_BIND_INDEX_BUFFER);
		D3D11_SUBRESOURCE_DATA iData = {};
		iData.pSysMem = indices.data();

		hResult = m_Device->CreateBuffer(&iDesc, &iData, pIndexBuffer.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | An error occured when creating the index buffer.");
		}
		
		float aspectRatio = (float)m_CurrentWindowDataRef.ClientArea.right / m_CurrentWindowDataRef.ClientArea.bottom;

		DXCamera camera(posX, posY, -15.0f, 45.0f, aspectRatio);
		camera.TranslatePos(offsetX, offsetY, offsetZ);

		// DirectX::XMMatrixTranslation(offsetX, offsetY, offsetZ) *
		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotAngleX)) * 
			DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotAngleY));

		CBTransform transformBuffer = {};
		transformBuffer.transform = DirectX::XMMatrixTranspose(
			worldMatrix * camera.ViewProjectionMatrix()
		);
		
		CD3D11_BUFFER_DESC cbTransformDesc((UINT)sizeof(CBTransform), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DEFAULT);
		D3D11_SUBRESOURCE_DATA cbTransformData = {};
		cbTransformData.pSysMem = &transformBuffer.transform;

		hResult = m_Device->CreateBuffer(&cbTransformDesc, &cbTransformData, pCBTransform.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | An error occured when creating the transform constant buffer.");
		}

		CBColors colors = {{
			{ 1.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 0.0f, 1.0f }
		}};

		CD3D11_BUFFER_DESC cbColorsDesc((UINT)sizeof(CBColors), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DEFAULT);
		D3D11_SUBRESOURCE_DATA cbColorsData = {};
		cbColorsData.pSysMem = &colors;

		hResult = m_Device->CreateBuffer(&cbColorsDesc, &cbColorsData, pCBColors.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | An error occured when creating the colors constant buffer.");
		}

		m_Device.ContextRaw()->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
		m_Device.ContextRaw()->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		m_Device.ContextRaw()->IASetInputLayout(pInputLayout.Get());

		m_Device.ContextRaw()->VSSetShader(shaderSet.VertexShader(), nullptr, 0);
		m_Device.ContextRaw()->PSSetShader(shaderSet.PixelShader(), nullptr, 0);

		m_Device.ContextRaw()->VSSetConstantBuffers(0, 1, pCBTransform.GetAddressOf());
		m_Device.ContextRaw()->PSSetConstantBuffers(0, 1, pCBColors.GetAddressOf());

		m_Device.ContextRaw()->DrawIndexed((UINT)indices.size(), 0, 0);	

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | Debug messages generated after drawing.");
			}
		);
	}*/

#pragma region Texture Attempts
#if 0
	/*
	void DXContext::TestTextureDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY, float offsetZ) noexcept
	{
		std::filesystem::path assetsPath = Utility::GetAssetsPath();

		if (!std::filesystem::exists(assetsPath))
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Assets directory doesn't exist.");

		std::filesystem::path texturePath = assetsPath / "textures/grassblock.png";

		if (!std::filesystem::exists(texturePath))
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Test texture doesn't exist.");

		std::wstring texturePathWStr = texturePath.wstring();



		/*Microsoft::WRL::ComPtr<ID3D11Resource> pTexture;*/
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;

		HRESULT hResult = DirectX::CreateWICTextureFromFile(
			m_Device.DeviceRaw(), 
			texturePathWStr.c_str(), 
			nullptr,
			pTextureView.GetAddressOf()
		);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Failed to create texture.");

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState;
		hResult = m_Device->CreateSamplerState(&samplerDesc, pSamplerState.GetAddressOf());

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Failed to create sampler.");

		struct Vertex {
			DirectX::XMFLOAT3 pos = {};
			DirectX::XMFLOAT2 texCoord = {};
		};
		
		struct CBTransform {
			DirectX::XMMATRIX transform = {};
		};

		Vertex vertices[] = { 
				// Front face
				{ { -1.0f, -1.0f, 1.0f }, { 0.16f * 2, 1.0f } },  // Bottom-left
				{ {  1.0f, -1.0f, 1.0f }, { 0.16f * 3, 1.0f } }, // Bottom-right
				{ { -1.0f,  1.0f, 1.0f }, { 0.16f * 2, 0.0f } },  // Top-left
				{ {  1.0f,  1.0f, 1.0f }, { 0.16f * 3, 0.0f } },  // Top-right

				// Back face
				{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f }  },  // Bottom-left
				{ {  1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f } },  // Bottom-right
				{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f }  },  // Top-left
				{ {  1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f } },  // Top-right

				// Top face
				{ { -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f } }, // Bottom-left
				{ {  1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f } }, // Bottom-right
				{ { -1.0f, 1.0f,  1.0f }, { 0.0f, 0.0f } }, // Top-left	
				{ {  1.0f, 1.0f,  1.0f }, { 0.0f, 0.0f } }, // Top-right

				// Bottom face
				{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f } }, // Bottom-left
				{ {  1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f } }, // Bottom-right
				{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f } }, // Top-left
				{ {  1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f } }, // Top-right

				//// Left face
				//{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f } }, // Bottom-left
				//{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f } }, // Bottom-right
				//{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f } }, // Top-left
				//{ { -1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f } }, // Top-right

				//// Right face
				//{ {  1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f } }, // Bottom-left
				//{ {  1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f } }, // Bottom-right
				//{ {  1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f } }, // Top-left
				//{ {  1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f } }, // Top-right
		};

		uint16_t indices[] = {
			2, 1, 0,
			2, 3, 1,

			4, 6, 5,
			6, 7, 5,

			/*8, 10, 9,
			10, 11, 9,

			12, 14, 13,
			14, 15, 13,
			
			16, 18, 17,
			18, 19, 17,

			20, 22, 21,
			22, 23, 21*/
		};

		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pTransformCB;

		CD3D11_BUFFER_DESC vBufferDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA vBufferData = {};
		vBufferData.pSysMem = vertices;

		CD3D11_BUFFER_DESC iBufferDesc(sizeof(indices), D3D11_BIND_INDEX_BUFFER);
		D3D11_SUBRESOURCE_DATA iBufferData = {};
		iBufferData.pSysMem = indices;

		hResult = m_Device->CreateBuffer(&vBufferDesc, &vBufferData, pVertexBuffer.GetAddressOf());

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Failed to create vertex buffer.");

		hResult = m_Device->CreateBuffer(&iBufferDesc, &iBufferData, pIndexBuffer.GetAddressOf());

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Failed to create index buffer.");

		float aspectRatio = (float)m_CurrentWindowDataRef.ClientArea.right / m_CurrentWindowDataRef.ClientArea.bottom;

		DXCamera camera(0.0f, 0.0f, -15.0f, 45.0f, aspectRatio);
		camera.TranslatePos(offsetX, offsetY, offsetZ);

		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotAngleX)) * DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotAngleY));

		CBTransform transformBuffer = {};
		transformBuffer.transform = DirectX::XMMatrixTranspose(
			worldMatrix * camera.ViewProjectionMatrix()
		);

		CD3D11_BUFFER_DESC cBufferDesc(sizeof(CBTransform), D3D11_BIND_CONSTANT_BUFFER);
		D3D11_SUBRESOURCE_DATA cBufferData = {};
		cBufferData.pSysMem = &transformBuffer.transform;

		hResult = m_Device->CreateBuffer(&cBufferDesc, &cBufferData, pTransformCB.GetAddressOf());

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Failed to create transform constant buffer.");



		DXShaderSet& shaderSet = m_ShaderLibrary.GetSetOfType(DXImplementedShaderType::DEFAULT_TEXTURE);

		if (!shaderSet.IsCreated())
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Shader set was not created previously.");

		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
		hResult = m_Device->CreateInputLayout(
			shaderSet.Desc().Data(),
			(UINT)shaderSet.Desc().Size(),
			shaderSet.VertexData().pBytecode->GetBufferPointer(),
			shaderSet.VertexData().pBytecode->GetBufferSize(),
			pInputLayout.GetAddressOf()
		);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Failed to create input layout.");
		
		UINT stride = sizeof(Vertex);
		UINT offset = 0;



		m_Device.ContextRaw()->IASetInputLayout(pInputLayout.Get());

		m_Device.ContextRaw()->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
		m_Device.ContextRaw()->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		m_Device.ContextRaw()->VSSetShader(shaderSet.VertexShader(), nullptr, 0);
		m_Device.ContextRaw()->PSSetShader(shaderSet.PixelShader(), nullptr, 0);

		m_Device.ContextRaw()->VSSetConstantBuffers(0, 1, pTransformCB.GetAddressOf());

		m_Device.ContextRaw()->PSSetShaderResources(0, 1, pTextureView.GetAddressOf());
		m_Device.ContextRaw()->PSSetSamplers(0, 1, pSamplerState.GetAddressOf());



		m_Device.Context()->OMSetRenderTargets(1, mP_RTV.GetAddressOf(), nullptr);

		m_Device.ContextRaw()->DrawIndexed((UINT)std::size(indices), 0, 0);

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Debug messages generated after drawing.");
			}
		);
	}*/
#else

	/*void DXContext::TestTextureDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY, float offsetZ) noexcept
	{
		struct Vertex {
			DirectX::XMFLOAT3 pos = {};
			//DirectX::XMFLOAT2 texCoord = {};
		};

		struct CBTransform {
			DirectX::XMMATRIX transform = {};
		};

		struct Color {
			float r, g, b, a;
		};

		struct CBColors {
			Color colors[6];
		};

		DXShaderSet& shaderSet = m_ShaderLibrary.GetSetOfType(DXImplementedShaderType::DEFAULT3D);

		if (!shaderSet.IsCreated())
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Shader set wasn't created previously.");

		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
		HRESULT hResult = m_Device->CreateInputLayout(
			shaderSet.Desc().Data(),
			(UINT)shaderSet.Desc().Size(),
			shaderSet.VertexData().pBytecode->GetBufferPointer(),
			shaderSet.VertexData().pBytecode->GetBufferSize(),
			&pInputLayout
		);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(L"DXContext [TestTextureDraw] | Failed to create input layout : ", WindowsUtility::TranslateDWORDError(hResult));

		float aspectRatio = (float)m_CurrentWindowDataRef.ClientArea.right / m_CurrentWindowDataRef.ClientArea.bottom;
		DXCamera camera(posX, posY, posZ, 45.0f, aspectRatio);

		Vertex vertices[] = {
			{ { -1.0f, -1.0f, -1.0f } },
			{ {  1.0f, -1.0f, -1.0f } },
			{ { -1.0f,  1.0f, -1.0f } },
		};

		uint16_t indices[] = {
			0, 2, 1,
		};

		CBColors colors = { {
			{ 1.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 0.0f, 1.0f }
		} };

		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(faceOneX, faceOneY, faceOneZ) * 
			DirectX::XMMatrixRotationX(faceOneRotX) * DirectX::XMMatrixRotationY(faceOneRotY);

		CBTransform mvp = {};
		mvp.transform = DirectX::XMMatrixTranspose(worldMatrix * camera.ViewProjectionMatrix());

		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pMVPBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pColorsBuffer;

		CD3D11_BUFFER_DESC vDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA vData = {};
		vData.pSysMem = vertices;

		CD3D11_BUFFER_DESC iDesc(sizeof(indices), D3D11_BIND_INDEX_BUFFER);
		D3D11_SUBRESOURCE_DATA iData = {};
		iData.pSysMem = indices;

		CD3D11_BUFFER_DESC mvpDesc(sizeof(CBTransform), D3D11_BIND_CONSTANT_BUFFER);
		D3D11_SUBRESOURCE_DATA mvpData = {};
		mvpData.pSysMem = &mvp.transform;

		CD3D11_BUFFER_DESC colorsDesc(sizeof(CBColors), D3D11_BIND_CONSTANT_BUFFER);
		D3D11_SUBRESOURCE_DATA colorsData = {};
		colorsData.pSysMem = &colors;

		hResult = m_Device->CreateBuffer(&vDesc, &vData, &pVertexBuffer);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(L"DXContext [TestTextureDraw] | Failed to create vertex buffer : ", WindowsUtility::TranslateDWORDError(hResult));

		hResult = m_Device->CreateBuffer(&iDesc, &iData, &pIndexBuffer);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(L"DXContext [TestTextureDraw] | Failed to create index buffer : ", WindowsUtility::TranslateDWORDError(hResult));

		hResult = m_Device->CreateBuffer(&mvpDesc, &mvpData, &pMVPBuffer);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(L"DXContext [TestTextureDraw] | Failed to create mvp constant buffer : ", WindowsUtility::TranslateDWORDError(hResult));

		hResult = m_Device->CreateBuffer(&colorsDesc, &colorsData, &pColorsBuffer);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(L"DXContext [TestTextureDraw] | Failed to create colors constant buffer : ", WindowsUtility::TranslateDWORDError(hResult));

		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;

		hResult = m_Device->CreateVertexShader(
			shaderSet.VertexData().pBytecode->GetBufferPointer(),
			shaderSet.VertexData().pBytecode->GetBufferSize(),
			nullptr,
			&pVertexShader
		);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(L"DXContext [TestTextureDraw] | Failed to create vertex shader : ", WindowsUtility::TranslateDWORDError(hResult));

		hResult = m_Device->CreatePixelShader(
			shaderSet.PixelData().pBytecode->GetBufferPointer(),
			shaderSet.PixelData().pBytecode->GetBufferSize(),
			nullptr,
			&pPixelShader
		);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(L"DXContext [TestTextureDraw] | Failed to create pixel shader : ", WindowsUtility::TranslateDWORDError(hResult));

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		m_Device.ContextRaw()->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
		m_Device.ContextRaw()->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		m_Device.ContextRaw()->IASetInputLayout(pInputLayout.Get());

		m_Device.ContextRaw()->VSSetShader(pVertexShader.Get(), nullptr, 0);
		m_Device.ContextRaw()->PSSetShader(pPixelShader.Get(), nullptr, 0);

		m_Device.ContextRaw()->VSSetConstantBuffers(0, 1, pMVPBuffer.GetAddressOf());
		m_Device.ContextRaw()->PSSetConstantBuffers(0, 1, pColorsBuffer.GetAddressOf());

		BindRTV();

		m_Device.ContextRaw()->DrawIndexed(std::size(indices), 0, 0);

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Debug messages generated after drawing.");
			}
		);

		worldMatrix = DirectX::XMMatrixTranslation(faceTwoX, faceTwoY, faceTwoZ) * 
			DirectX::XMMatrixRotationX(faceTwoRotX) * DirectX::XMMatrixRotationY(faceTwoRotY);

		mvp.transform = DirectX::XMMatrixTranspose(worldMatrix * camera.ViewProjectionMatrix());
		mvpData.pSysMem = &mvp.transform;

		hResult = m_Device->CreateBuffer(&mvpDesc, &mvpData, &pMVPBuffer);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(L"DXContext [TestTextureDraw] | Failed to re-create mvp constant buffer : ", WindowsUtility::TranslateDWORDError(hResult));

		m_Device.ContextRaw()->VSSetConstantBuffers(0, 1, pMVPBuffer.GetAddressOf());

		m_Device.ContextRaw()->DrawIndexed(std::size(indices), 0, 0);

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_CMLoggerRef.LogFatalNL(L"DXContext [TestTextureDraw] | Debug messages generated after drawing.");
			}
		);
	}*/
#endif
#pragma endregion
#pragma endregion

	void DXContext::InitImGui(const HWND hWnd) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Device.IsCreated(), L"DXContext [InitImGui] | Device was not created previously.");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(m_Device.DeviceRaw(), m_Device.ContextRaw());
	}

	void DXContext::ShutdownImGui() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"DXContext [InitImGui] | Context was not initialized previously.");

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void DXContext::CreateRTV() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(mP_RTV.Get() != nullptr, L"DXContext [CreateRTV] | A RTV is still present.");
		m_CMLoggerRef.LogFatalNLIf(mP_DSV.Get() != nullptr, L"DXContext [CreateRTV] | A DSV is still present.");

		// Get the back buffer.
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		HRESULT hResult = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		m_CMLoggerRef.LogFatalNLAppendIf(
			FAILED(hResult),
			L"DXContext [CreateRTV] | Failed to get back buffer : ",
			WindowsUtility::TranslateDWORDError(hResult)
		);

		// Create the RTV.
		hResult = m_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, mP_RTV.GetAddressOf());

		m_CMLoggerRef.LogFatalNLAppendIf(
			FAILED(hResult),
			L"DXContext [CreateRTV] | Failed to create render target view : ",
			WindowsUtility::TranslateDWORDError(hResult)
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

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(
				L"DXContext [CreateRTV] | Failed to create depth stencil texture : ",
				WindowsUtility::TranslateDWORDError(hResult)
			);

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable = FALSE;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
		hResult = m_Device->CreateDepthStencilState(&depthStencilDesc, pDSState.GetAddressOf());

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(
				L"DXContext [CreateRTV] | Failed to create depth stencil state : ",
				WindowsUtility::TranslateDWORDError(hResult)
			);

		m_Device.ContextRaw()->OMSetDepthStencilState(pDSState.Get(), 1u);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0u;

		hResult = m_Device->CreateDepthStencilView(pDepthStencilTexture.Get(), &dsvDesc, mP_DSV.GetAddressOf());

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(
				L"DXContext [CreateRTV] | Failed to create depth stencil view : ",
				WindowsUtility::TranslateDWORDError(hResult)
			);
	}

	void DXContext::BindRTV() noexcept
	{
		m_Device.ContextRaw()->OMSetRenderTargets(1u, mP_RTV.GetAddressOf(), mP_DSV.Get());
	}

	void DXContext::SetViewport() noexcept
	{
		CD3D11_VIEWPORT viewport(
			0.0f, 
			0.0f, 
			static_cast<FLOAT>(m_State.CurrentClientArea().right),
			static_cast<FLOAT>(m_State.CurrentClientArea().bottom)
		);

		m_Device.ContextRaw()->RSSetViewports(1, &viewport);
	}

	void DXContext::SetTopology() noexcept
	{
		m_Device.ContextRaw()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void DXContext::ResetState() noexcept
	{
		m_Device.ContextRaw()->ClearState();
		m_Device.ContextRaw()->Flush();

		mP_RTV.Reset();
		mP_DSV.Reset();
	}

	void DXContext::OnWindowResize() noexcept
	{
		m_State.UpdateResolution();

		if (!m_Initialized)
			return;

		ResetState();

		SetViewport();
		SetTopology();

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		HRESULT hResult = m_SwapChain->GetDesc(&swapChainDesc);

		m_CMLoggerRef.LogFatalNLIf(FAILED(hResult), L"DXContext [OnWindowResize] | Failed to retrieve swap chain description.");

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

		m_CMLoggerRef.LogFatalNLIf(FAILED(hResult), L"DXContext [OnWindowResize] | Failed to resize buffers.");

		CreateRTV();
		BindRTV();

		/* Update the frame data constant buffer. */
		CMFrameData frameData = {
			static_cast<float>(clientArea.right),
			static_cast<float>(clientArea.bottom),
			{ 0.0f, 0.0f }
		};

		m_CMLoggerRef.LogFatalNLIf(!m_CBFrameData.IsCreated(), L"DXContext [OnWindowResize] | CMFrameData constant buffer wasn't created previously.");

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		hResult = m_Device.ContextRaw()->Map(m_CBFrameData.Buffer(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);

		m_CMLoggerRef.LogFatalNLIf(FAILED(hResult), L"DXContext [OnWindowResize] | Failed to update FrameData constant buffer.");

		memcpy(mappedResource.pData, &frameData, sizeof(CMFrameData));
		
		m_Device.ContextRaw()->Unmap(m_CBFrameData.Buffer(), 0u);

		RebindCurrentShaderSet();
	}

	void DXContext::RebindCurrentShaderSet() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Initialized, L"DXContext [RebindCurrentShaderSet] | Context isn't initialized.");

		m_State.RebindCurrentShaderSet();
	}
#pragma endregion
}