#include "PCH.hpp"
#include "Platform/WinImpl/Platform_WinImpl.hpp"
#include "Platform/WinImpl/Graphics_WinImpl.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"
#include "Platform/WinImpl/GPUBuffer_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
	struct Vertex
	{
		aiVector3D Pos;
		aiVector3D Normal;
		aiVector3D TexCoord;
	};

	struct alignas(16) TransformCB
	{
		static constexpr UINT S_VS_REGISTER_SLOT = 0;

		DirectX::XMMATRIX Model;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Proj;
	};

	struct alignas(16) MaterialCB
	{
		static constexpr UINT S_PS_REGISTER_SLOT = 0;

		aiColor4D BaseColor;
		float Metallic = 0.0f;
		float Roughness = 0.0f;
		float padding[2] = { 0.0f, 0.0f };
	};

	struct alignas(16) CameraCB
	{
		static constexpr UINT S_PS_REGISTER_SLOT = 1;

		Float3 CameraPos;
		float padding;
	};

	struct SphereInstanceVB
	{
		static constexpr UINT S_REGISTER_SLOT = 1;

		Float3 Origin;
		float Radius = 0.0f;
	};

	static constexpr Float2 S_QUAD_FRONT_VERTICES[] = {
			{ -0.5f,  0.5f },
			{  0.5f,  0.5f },
			{  0.5f, -0.5f },
			{ -0.5f, -0.5f }
	};

	static constexpr uint16_t S_QUAD_FRONT_INDICES[] = {
		0, 1, 2,
		2, 3, 0
	};

	Graphics::Graphics(Window& window, const PlatformConfig& platformConfig) noexcept
		: m_Window(window),
		  m_Config(platformConfig)
	{
		Init();
		m_Window.SetCallbackOnResize(OnResizeThunk, this);
	}

	Graphics::~Graphics() noexcept
	{
		m_Window.RemoveCallbackOnResize(OnResizeThunk, this);
		Shutdown();
	}

	//void Graphics::Update() noexcept
	//{
	//	ImGui_ImplWin32_NewFrame();
	//	ImGui_ImplDX11_NewFrame();
	//	ImGui::NewFrame();

	//	if (ImGui::Begin("Engine Control"))
	//	{
	//		if (ImGui::CollapsingHeader("Camera"))
	//			ImGui::SliderFloat3("Offset", m_CameraOffset.Underlying(), -20.0f, 20.0f);

	//		if (ImGui::CollapsingHeader("Mesh"))
	//			ImGui::SliderFloat3("Offset##xx", m_MeshOffset.Underlying(), -20.0f, 20.0f);

	//		if (ImGui::CollapsingHeader("Text"))
	//		{
	//			ImGui::SliderFloat2("Offset##xx##xx", m_TextOffset.Underlying(), 0.0f, 400.0f);
	//			ImGui::SliderFloat2("Resolution", m_TextResolution.Underlying(), 0.0f, 400.0f);

	//			if (m_ShowTextBounds)
	//				ImGui::SliderFloat4("Bounds RGBA", m_TextBoundsRGBA.rgba, 0.0f, 1.0f);

	//			ImGui::Checkbox("Show Bounds?", &m_ShowTextBounds);
	//		}
	//	}

	//	ImGui::End();

	//	const aiScene* pModel = mP_ModelImporter->Assimp.ReadFile(
	//		CM_ENGINE_RESOURCES_MODEL_DIRECTORY "/cube.gltf",
	//		aiProcess_Triangulate |
	//		aiProcess_JoinIdenticalVertices |
	//		aiProcess_ConvertToLeftHanded
	//	);

	//	if (!pModel || !pModel->mMeshes[0])
	//	{
	//		spdlog::critical(
	//			"(WinImpl_Graphics) Internal error: Error occured loading model. "
	//			"Error: {}", mP_ModelImporter->Assimp.GetErrorString()
	//		);

	//		std::exit(-1);
	//	}

	//	const aiMesh* pMesh = pModel->mMeshes[0];

	//	std::vector<Vertex> vertices;
	//	vertices.reserve(pMesh->mNumVertices);

	//	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
	//	{
	//		aiVector3D texCoord = pMesh->HasTextureCoords(0)
	//			? pMesh->mTextureCoords[0][i]  // UV channel 0
	//			: aiVector3D(0.0f, 0.0f, 0.0f); // fallback

	//		vertices.emplace_back(pMesh->mVertices[i], pMesh->mNormals[i], texCoord);
	//	}

	//	/* Since we triangulated, each face should be a triangle (i.e., require 3 indices. ex. 0, 1, 2). */
	//	UINT numIndices = pMesh->mNumFaces * 3;

	//	std::vector<uint16_t> indices;
	//	indices.reserve(numIndices);

	//	for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
	//	{
	//		const auto& face = pMesh->mFaces[i];
	//		CM_ENGINE_ASSERT(face.mNumIndices == 3);

	//		indices.emplace_back(face.mIndices[0]);
	//		indices.emplace_back(face.mIndices[1]);
	//		indices.emplace_back(face.mIndices[2]);
	//	}

	//	const aiMaterial* pMaterial = pModel->mMaterials[pMesh->mMaterialIndex];
	//	MaterialCB materialData = {};

	//	aiColor4D baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	//	if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_BASE_COLOR, &baseColor))
	//		materialData.BaseColor = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };

	//	float metallic = 0.0f;
	//	if (AI_SUCCESS == aiGetMaterialFloat(pMaterial, AI_MATKEY_METALLIC_FACTOR, &metallic))
	//		materialData.Metallic = metallic;

	//	float roughness = 0.0f;
	//	if (AI_SUCCESS == aiGetMaterialFloat(pMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &roughness))
	//		materialData.Roughness = roughness;

	//	VertexBuffer vbVertices(sizeof(Vertex));
	//	vbVertices.Create(vertices.data(), vertices.size() * sizeof(Vertex), mP_Device);
	//	vbVertices.Upload(mP_Context);

	//	IndexBuffer ibIndices;
	//	ibIndices.Create(indices.data(), indices.size() * sizeof(uint16_t), mP_Device);
	//	ibIndices.Upload(mP_Context);

	//	ConstantBuffer cbMaterial(ConstantBufferType::PS, MaterialCB::S_PS_REGISTER_SLOT);
	//	cbMaterial.Create(std::span<const MaterialCB>(&materialData, 1), mP_Device);
	//	cbMaterial.Upload(mP_Context);

	//	CameraCB cameraData = {};
	//	cameraData.CameraPos = m_CameraOffset;

	//	ConstantBuffer cbCamera(ConstantBufferType::PS, CameraCB::S_PS_REGISTER_SLOT);
	//	cbCamera.Create(std::span<const CameraCB>(&cameraData, 1), mP_Device);
	//	cbCamera.Upload(mP_Context);

	//	float aspectRatio = m_Window.ClientResolution().Aspect();
	//	constexpr float CameraFovDeg = 45.0f;
	//	constexpr float CameraFovRad = DirectX::XMConvertToRadians(CameraFovDeg);

	//	DirectX::XMFLOAT3 cameraPos = ToXMFloat3(m_CameraOffset);
	//	DirectX::XMFLOAT3 cameraFocus = {};
	//	DirectX::XMFLOAT3 upDirection = { 0.0f, 1.0f, 0.0f };

	//	DirectX::XMVECTOR cameraPosVec = DirectX::XMLoadFloat3(&cameraPos);
	//	DirectX::XMVECTOR cameraFocusVec = DirectX::XMLoadFloat3(&cameraFocus);
	//	DirectX::XMVECTOR upDirectionVec = DirectX::XMLoadFloat3(&upDirection);

	//	TransformCB transformData;
	//	transformData.Model = DirectX::XMMatrixTranspose(
	//		DirectX::XMMatrixTranslation(m_MeshOffset.x, m_MeshOffset.y, m_MeshOffset.z)
	//	);

	//	transformData.View = DirectX::XMMatrixTranspose(
	//		DirectX::XMMatrixLookAtLH(cameraPosVec, cameraFocusVec, upDirectionVec)
	//	);

	//	transformData.Proj = DirectX::XMMatrixTranspose(
	//		DirectX::XMMatrixPerspectiveFovLH(CameraFovRad, aspectRatio, 0.05f, 100.0f)
	//	);

	//	ConstantBuffer cbTransform(ConstantBufferType::VS, TransformCB::S_VS_REGISTER_SLOT);
	//	cbTransform.Create(std::span<const TransformCB>(&transformData, 1), mP_Device);
	//	cbTransform.Upload(mP_Context);

	//	Clear(Color::Black());

	//	m_ShaderLibrary.BindSet(ShaderSetType::GLTF, mP_Context);
	//	mP_Context->DrawIndexed(numIndices, 0, 0);

	//	D2DBeginDraw();

	//	if (m_ShowTextBounds)
	//		D2DDrawRect(m_TextOffset, m_TextResolution, Color::White());

	//	constexpr std::wstring_view TestText = L"Hello Direct2D & DirectWrite!";
	//	D2DDrawText(TestText, m_TextOffset, m_TextResolution, Color::White());

	//	D2DEndDraw();

	//	ImGui::Render();
	//	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	//	ImGuiIO& io = ImGui::GetIO();
	//	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//	{
	//		ImGui::UpdatePlatformWindows();
	//		ImGui::RenderPlatformWindowsDefault();
	//	}

	//	Present();
	//}

	void Graphics::Clear(const Color4& color) noexcept
	{
		mP_Context->ClearRenderTargetView(mP_RTV.Get(), color.rgba);
		mP_Context->ClearDepthStencilView(mP_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void Graphics::Present() noexcept
	{
		HRESULT hr = mP_SwapChain->Present(m_PresentSyncInterval, m_PresentFlags);

		if (!FAILED(hr))
		{
			BindViews();
			return;
		}

		if (hr == DXGI_ERROR_DEVICE_REMOVED)
			spdlog::critical("(WinImpl_Graphics) Internal error: Device removed. Reason: {}", mP_Device->GetDeviceRemovedReason());
		else
			spdlog::critical("(WinImpl_Graphics) Internal error: Error occured after presenting. Error code: {}", hr);

		if (m_LoadedDebugLayer)
		{
			if (mP_InfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL) == 0)
				return;

			spdlog::warn("(WinImpl_Graphics) Internal warning: Debug messages generated after presenting.");
		}
	}

	void Graphics::Draw(const void* pBuffer, const DrawDescriptor& descriptor) noexcept
	{
	}

	void Graphics::StartFrame(const Color4& clearColor) noexcept
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		Clear(clearColor);

		m_IsWithinFrame = true;
	}

	void Graphics::EndFrame() noexcept
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		Present();

		m_IsWithinFrame = false;
	}

	[[nodiscard]] IGraphics::BufferPtr<IUploadable> Graphics::CreateBuffer(GPUBufferType type, GPUBufferFlag flags) noexcept
	{
		switch (type)
		{
		case GPUBufferType::Invalid: [[fallthrough]];
		default:
			return BufferPtr<IUploadable>(nullptr);
		case GPUBufferType::Vertex:
			return BufferPtr<VertexBuffer>(new VertexBuffer(flags));
		case GPUBufferType::Index:
			return BufferPtr<IndexBuffer>(new IndexBuffer(flags));
		case GPUBufferType::Constant:
			return BufferPtr<ConstantBuffer>(new ConstantBuffer(flags));
		}
	}

	void Graphics::SetBuffer(const Graphics::BufferPtr<IUploadable>& pBuffer, void* pData, size_t numBytes) noexcept
	{
		IGPUBuffer* pDerived = dynamic_cast<IGPUBuffer*>(pBuffer.get());

		if (!pDerived)
		{
			spdlog::warn("(WinImpl_Graphics) [SetConstantBuffer] Internal warning: Attempted to set an object that was either nullptr, or not of type derived from IGPUBuffer.");
			return;
		}

		if (pDerived->IsCreated() && pDerived->HasFlag(GPUBufferFlag::Dynamic))
			pDerived->Update(pData, numBytes, mP_Context);
		else
			pDerived->Create(pData, numBytes, mP_Device);
	}

	void Graphics::BindVertexBuffer(const Graphics::BufferPtr<IUploadable>& pBuffer, UINT strideBytes, UINT offsetBytes, UINT slot) noexcept
	{
		VertexBuffer* pDerivedVB = dynamic_cast<VertexBuffer*>(pBuffer.get());

		if (!pDerivedVB)
		{
			spdlog::warn("(WinImpl_Graphics) [BindConstantBufferVS] Internal warning: Attempted to bind a buffer instance that was either nullptr, or not of type VertexBuffer.");
			return;
		}

		pDerivedVB->SetStride(strideBytes);
		pDerivedVB->SetOffset(offsetBytes);
		pDerivedVB->SetRegister(slot);
		pDerivedVB->Upload(mP_Context);
	}

	void Graphics::BindIndexBuffer(const Graphics::BufferPtr<IUploadable>& pBuffer, DXGI_FORMAT format, UINT startIndex) noexcept
	{
		IndexBuffer* pDerivedIB = dynamic_cast<IndexBuffer*>(pBuffer.get());

		if (!pDerivedIB)
		{
			spdlog::warn("(WinImpl_Graphics) [BindConstantBufferVS] Internal warning: Attempted to bind a buffer instance that was either nullptr, or not of type IndexBuffer.");
			return;
		}

		pDerivedIB->SetFormat(format);
		pDerivedIB->SetOffset(startIndex);
		pDerivedIB->Upload(mP_Context);
	}

	[[nodiscard]] void Graphics::BindConstantBufferVS(const Graphics::BufferPtr<IUploadable>& pBuffer, UINT slot) noexcept
	{
		ConstantBuffer* pDerivedCB = dynamic_cast<ConstantBuffer*>(pBuffer.get());

		if (!pDerivedCB)
		{
			spdlog::warn("(WinImpl_Graphics) [BindConstantBufferVS] Internal warning: Attempted to bind a buffer instance that was either nullptr, or not of type ConstantBuffer.");
			return;
		}

		pDerivedCB->SetType(ConstantBufferType::VS);
		pDerivedCB->SetRegister(slot);
		pDerivedCB->Upload(mP_Context);
	}

	[[nodiscard]] void Graphics::BindConstantBufferPS(const Graphics::BufferPtr<IUploadable>& pBuffer, UINT slot) noexcept
	{
		ConstantBuffer* pDerivedCB = dynamic_cast<ConstantBuffer*>(pBuffer.get());

		if (!pDerivedCB)
		{
			spdlog::warn("(WinImpl_Graphics) [BindConstantBufferPS] Internal warning: Attempted to bind a buffer instance that was either nullptr, or not of type ConstantBuffer.");
			return;
		}

		pDerivedCB->SetType(ConstantBufferType::PS);
		pDerivedCB->SetRegister(slot);
		pDerivedCB->Upload(mP_Context);
	}

	void Graphics::Init() noexcept
	{
		if (m_Config.IsGraphicsDebugging)
			spdlog::info("(WinImpl_Graphics) Internal info: Graphics debugger is active, so Direct2D and DirectWrite resources will not be initialized.");

		DXGI_SWAP_CHAIN_DESC scDesc = {};
		scDesc.BufferDesc.Width = 0;
		scDesc.BufferDesc.Height = 0;
		scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scDesc.BufferDesc.RefreshRate.Numerator = 0;
		scDesc.BufferDesc.RefreshRate.Denominator = 0;
		scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scDesc.SampleDesc.Count = 1; // Count 1 and Quality 0 disables anti-aliasing.
		scDesc.SampleDesc.Quality = 0;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = 2;
		scDesc.OutputWindow = m_Window.Impl_HWND();
		scDesc.Windowed = true;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.Flags = 0;

		/* NOTE: D3D11_CREATE_DEVICE_BGRA_SUPPORT is required for D3D11 - D2D interop. */
		UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		/* NOTE: If D3D11_CREATE_DEVICE_DEBUG is not provided, the debug layer will not be able
		 *		   to be loaded. */
		CM_ENGINE_IF_DEBUG(flags |= D3D11_CREATE_DEVICE_DEBUG);
		CM_ENGINE_IF_NDEBUG(
			if (m_Config.IsGraphicsDebugging)
			{
				spdlog::info("(WinImpl_Graphics) Internal info: Creating device with debug layer in non-debug build config since a graphics debugger is active.");
				flags |= D3D11_CREATE_DEVICE_DEBUG;
			}
		);

		spdlog::info("(WinImpl_Graphics) Internal info: Creating device with flags: {}", flags);
		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&scDesc,
			&mP_SwapChain,
			&mP_Device,
			nullptr,
			&mP_Context
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create device and swap chain. Error code: {}", hr);

		if (CM_ENGINE_IS_DEBUG || m_Config.IsGraphicsDebugging)
		{
			HMODULE pDxgiDebugModule = GetModuleHandleW(L"Dxgidebug.dll");

			if (pDxgiDebugModule == nullptr)
				spdlog::warn("(WinImpl_Graphics) Internal warning: Failed to get Dxgidebug.dll module. Debug info will not be available. (Graphics Debugger interference?)");
			else
			{
				spdlog::info("(WinImpl_Graphics) Internal info: Successfuly loaded Dxgidebug.dll modudle.");

				using GetDXGIInfoQueueFunc = HRESULT(*)(REFIID, void**);
				GetDXGIInfoQueueFunc pDXGIGetDebugInterfaceFunc = reinterpret_cast<GetDXGIInfoQueueFunc>(GetProcAddress(pDxgiDebugModule, "DXGIGetDebugInterface"));

				hr = pDXGIGetDebugInterfaceFunc(IID_PPV_ARGS(&mP_InfoQueue));

				if (FAILED(hr))
					spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get DXGI info queue. Error code: {}", hr);

				hr = pDXGIGetDebugInterfaceFunc(IID_PPV_ARGS(&mP_DebugInterface));

				if (FAILED(hr))
					spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get DXGIDebug interface. Error code: {}", hr);

				m_LoadedDebugLayer = true;
			}
		}

		InitDWrite();

		CreateViews();
		BindViews();

		mP_SwapChain->GetDesc(&scDesc);

		Float2 resolution(
			static_cast<float>(scDesc.BufferDesc.Width),
			static_cast<float>(scDesc.BufferDesc.Height)
		);

		SetViewport(resolution);

		m_ShaderLibrary.CreateShaderSets(mP_Device);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // IF using Docking Branch

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(m_Window.Impl_HWND());
		ImGui_ImplDX11_Init(mP_Device.Get(), mP_Context.Get());
	}

	void Graphics::Shutdown() noexcept
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void Graphics::CreateViews() noexcept
	{
		ComPtr<ID3D11Texture2D> pBackBuffer;
		HRESULT hr = mP_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to retrieve back buffer. Error code: {}", hr);

		hr = mP_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &mP_RTV);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create render target view. Error code: {}", hr);

		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		ComPtr<ID3D11DepthStencilState> pDSState;
		hr = mP_Device->CreateDepthStencilState(&dsDesc, &pDSState);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create depth stencil state. Error code: {}", hr);

		mP_Context->OMSetDepthStencilState(pDSState.Get(), 1);

		D3D11_TEXTURE2D_DESC backBufferSurfaceDesc = {};
		pBackBuffer->GetDesc(&backBufferSurfaceDesc);

		ComPtr<ID3D11Texture2D> pDSTexture;
		D3D11_TEXTURE2D_DESC descDepth = {};
		descDepth.Width = backBufferSurfaceDesc.Width;
		descDepth.Height = backBufferSurfaceDesc.Height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D32_FLOAT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = mP_Device->CreateTexture2D(&descDepth, nullptr, &pDSTexture);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create depth stencil buffer. Error code: {}", hr);

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
		descDSV.Format = DXGI_FORMAT_UNKNOWN;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		hr = mP_Device->CreateDepthStencilView(pDSTexture.Get(), &descDSV, &mP_DSV);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create depth stencil view. Error code: {}", hr);

		CreateD2DViews(pBackBuffer);
	}

	void Graphics::BindViews() noexcept
	{
		mP_Context->OMSetRenderTargets(1, mP_RTV.GetAddressOf(), mP_DSV.Get());
	}

	void Graphics::ReleaseViews() noexcept
	{
		mP_RTV.Reset();
		mP_DSV.Reset();

		ReleaseD2DViews();
	}

	void Graphics::SetViewport(Float2 resolution) noexcept
	{
		CD3D11_VIEWPORT viewport(
			0.0f, 0.0f, resolution.x, resolution.y
		);

		mP_Context->RSSetViewports(1, &viewport);

		mP_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void Graphics::OnResizeCallback(Float2 resolution) noexcept
	{
		mP_Context->ClearState();
		mP_Context->Flush();

		ReleaseViews();

		DXGI_SWAP_CHAIN_DESC swapDesc;
		HRESULT hr = mP_SwapChain->GetDesc(&swapDesc);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get swap chain desc.");
		
		/* If a view is bound to a deferred context, you must discard
		 *   the partially built command list as well (by calling ID3D11DeviceContext::ClearState,
		 *   then ID3D11DeviceContext::FinishCommandList, then Release on the command list). */
		hr = mP_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, swapDesc.Flags);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to resize buffers.");

		CreateViews();
		BindViews();

		SetViewport(resolution);
	}

	void Graphics::OnResizeThunk(Float2 resolution, void* pThis) noexcept
	{
		reinterpret_cast<Graphics*>(pThis)->OnResizeCallback(resolution);
	}

	[[nodiscard]] bool Graphics::IsGraphicsDebugging() const noexcept
	{
		/* A graphics debugger can interfere with creation of Direct2D and DWrite resources,
		 *   so refrain from using such resources while the process is hooked into by a
		 *   graphics debugger. */
		return m_Config.IsGraphicsDebugging;
	}

	void Graphics::InitDWrite() noexcept
	{
		if (IsGraphicsDebugging())
			return;

		ComPtr<IDXGISurface> pBackBufferSurface;
		HRESULT hr = mP_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBufferSurface));

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to retrieve back buffer. Error code: {}", hr);

		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, mP_D2D_Factory.GetAddressOf());

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create Direct2D factory. Error code: {}", hr);

		/* TODO: Research the distinction between DWRITE_FACTORY_TYPE_SHARED and DWRITE_FACTORY_TYPE_ISOLATED. */
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(mP_DW_Factory.GetAddressOf())
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create DirectWrite factory. Error code: {}", hr);

		hr = mP_DW_Factory->CreateTextFormat(
			L"Gabriola",                // Font family name.
			NULL,                       // Font collection (NULL sets it to use the system font collection).
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			24.0f,
			L"en-us",
			&mP_DW_TextFormat
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create text format. Error code: {}", hr);

		hr = mP_DW_TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to set text alignment. Error code: {}", hr);

		hr = mP_DW_TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to set paragraph alignment. Error code: {}", hr);
	}

	void Graphics::CreateD2DViews(const ComPtr<ID3D11Texture2D>& pBackBuffer) noexcept
	{
		if (IsGraphicsDebugging())
			return;

		ComPtr<IDXGISurface> pBackBufferSurface;
		HRESULT hr = pBackBuffer.As(&pBackBufferSurface);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to retrieve back buffer as IDXGISurface. Error code: {}", hr);

		hr = mP_D2D_Factory->CreateDxgiSurfaceRenderTarget(
			pBackBufferSurface.Get(),
			D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
			),
			&mP_D2D_RT
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create ID2D1RenderTarget from back buffer. Error code: {}", hr);

		hr = mP_D2D_RT->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			&mP_D2D_SC_Brush
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create ID2D1SolidColorBrush. Error code: {}", hr);
	}

	void Graphics::ReleaseD2DViews() noexcept
	{
		if (IsGraphicsDebugging())
			return;

		mP_D2D_SC_Brush.Reset();
		mP_D2D_RT.Reset();
	}

	void Graphics::D2DBeginDraw() noexcept
	{
		if (IsGraphicsDebugging())
			return;

		mP_D2D_RT->BeginDraw();
	}

	void Graphics::D2DEndDraw() noexcept
	{
		if (IsGraphicsDebugging())
			return;

		mP_D2D_RT->EndDraw();
	}

	void Graphics::D2DDrawText(const std::wstring_view& text, const Float2& pos, const Float2& resolution, const Color4& color) noexcept
	{
		if (IsGraphicsDebugging())
			return;

		D2D1_RECT_F layoutRect = ToD2D1RectF(pos, resolution);

		mP_D2D_SC_Brush->SetColor(ToD2D1ColorF(color));

		mP_D2D_RT->DrawText(
			text.data(),
			static_cast<UINT32>(text.length()),
			mP_DW_TextFormat.Get(),
			layoutRect,
			mP_D2D_SC_Brush.Get()
		);
	}

	void Graphics::D2DDrawRect(const Float2& pos, const Float2& resolution, const Color4& color) noexcept
	{
		if (IsGraphicsDebugging())
			return;

		D2D1_RECT_F layoutRect = ToD2D1RectF(pos, resolution);

		mP_D2D_SC_Brush->SetColor(ToD2D1ColorF(color));

		mP_D2D_RT->DrawRectangle(layoutRect, mP_D2D_SC_Brush.Get());
	}
}