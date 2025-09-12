#include "PCH.hpp"
#include "Platform/WinImpl/Graphics_WinImpl.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"
#include "Platform/WinImpl/GPUBuffer_WinImpl.hpp"


namespace CMEngine::Platform::WinImpl
{
	Graphics::Graphics(Window& window) noexcept
		: m_Window(window)
	{
		Init();
		m_Window.SetCallbackOnResize(OnResizeThunk, this);
	}

	Graphics::~Graphics() noexcept
	{
		m_Window.RemoveCallbackOnResize(OnResizeThunk, this);
		Shutdown();
	}

	void Graphics::Update() noexcept
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		if (ImGui::Begin("Engine Control"))
		{
			if (ImGui::CollapsingHeader("Camera"))
				ImGui::SliderFloat3("Offset", m_CameraOffset.Underlying(), -20.0f, 20.0f);

			if (ImGui::CollapsingHeader("Mesh"))
				ImGui::SliderFloat3("Offset##xx", m_MeshOffset.Underlying(), -20.0f, 20.0f);

			if (ImGui::CollapsingHeader("Text"))
			{
				ImGui::SliderFloat2("Offset##xx##xx", m_TextOffset.Underlying(), 0.0f, 400.0f);
				ImGui::SliderFloat2("Resolution", m_TextResolution.Underlying(), 0.0f, 400.0f);

				if (m_ShowTextBounds)
					ImGui::SliderFloat4("Bounds RGBA", m_TextBoundsRGBA.rgba, 0.0f, 1.0f);

				ImGui::Checkbox("Show Bounds?", &m_ShowTextBounds);
			}
		}

		ImGui::End();

		Float3 vertices[] = {
			{ -0.5f,  0.5f, 1.0f },
			{  0.5f,  0.5f, 1.0f },
			{  0.5f, -0.5f, 1.0f },
			{ -0.5f, -0.5f, 1.0f }
		};

		uint16_t indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		VertexBuffer vbVertices(sizeof(Float3));
		vbVertices.Create(vertices, sizeof(vertices), mP_Device);
		vbVertices.Upload(mP_Context);

		IndexBuffer ibIndices;
		ibIndices.Create(indices, sizeof(indices), mP_Device);
		ibIndices.Upload(mP_Context);

		float aspectRatio = m_Window.ClientResolution().Aspect();
		constexpr float CameraFovDeg = 45.0f;
		constexpr float CameraFovRad = DirectX::XMConvertToRadians(CameraFovDeg);

		DirectX::XMFLOAT3 cameraPos = ToXMFloat3(m_CameraOffset);
		DirectX::XMFLOAT3 cameraFocus = {};
		DirectX::XMFLOAT3 upDirection = { 0.0f, 1.0f, 0.0f };

		DirectX::XMVECTOR cameraPosVec = DirectX::XMLoadFloat3(&cameraPos);
		DirectX::XMVECTOR cameraFocusVec = DirectX::XMLoadFloat3(&cameraFocus);
		DirectX::XMVECTOR upDirectionVec = DirectX::XMLoadFloat3(&upDirection);

		DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixTranslation(m_MeshOffset.x, m_MeshOffset.y, m_MeshOffset.z);
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(cameraPosVec, cameraFocusVec, upDirectionVec);
		DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(CameraFovRad, aspectRatio, 0.05f, 100.0f);

		DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(modelMatrix * viewMatrix * projMatrix);

		ConstantBuffer cbMvpMatrix(ConstantBufferType::VS);
		cbMvpMatrix.Create(&mvpMatrix, sizeof(mvpMatrix), mP_Device);
		cbMvpMatrix.Upload(mP_Context);

		Clear(RGBANorm::Black());

		mP_Context->DrawIndexed(std::size(indices), 0, 0);

		mP_D2D_RT->BeginDraw();

		D2D1_RECT_F layoutRect = D2D1::RectF(
			m_TextOffset.x,
			m_TextOffset.y,
			m_TextResolution.x + m_TextOffset.x,
			m_TextResolution.y + m_TextOffset.y
		);

		mP_SC_Brush->SetColor(
			D2D1::ColorF(
				m_TextBoundsRGBA.r(),
				m_TextBoundsRGBA.g(),
				m_TextBoundsRGBA.b(),
				m_TextBoundsRGBA.a()
			)
		);

		if (m_ShowTextBounds)
			mP_D2D_RT->DrawRectangle(layoutRect, mP_SC_Brush.Get(), 1.0f, nullptr);

		mP_SC_Brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));

		constexpr std::wstring_view TestText = L"Hello Direct2D & DirectWrite!";

		mP_D2D_RT->DrawText(
			TestText.data(),
			static_cast<UINT32>(TestText.length()),
			mP_TextFormat.Get(),
			layoutRect,
			mP_SC_Brush.Get()
		);

		mP_D2D_RT->EndDraw();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		Present();
	}

	void Graphics::Clear(RGBANorm color) noexcept
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
			spdlog::critical("(WinImpl_Graphics) Internal error: Errer occured after presenting. Error code: {}", hr);

		CM_ENGINE_IF_DEBUG(
			if (mP_InfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL) == 0)
				return;

			spdlog::warn("(WinImpl_Graphics) Internal warning: Debug messages generated after presenting.");
			);
	}

	void Graphics::Draw(const void* pBuffer, const DrawDescriptor& descriptor) noexcept
	{
	}

	void Graphics::Init() noexcept
	{
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

		CM_ENGINE_IF_DEBUG(
			HMODULE pDxgiDebugModule = GetModuleHandleW(L"Dxgidebug.dll");

		if (pDxgiDebugModule == nullptr)
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get Dxgidebug.dll module.");
			std::exit(-1);
		}

		using GetDXGIInfoQueueFunc = HRESULT(*)(REFIID, void**);
		GetDXGIInfoQueueFunc pDXGIGetDebugInterfaceFunc = reinterpret_cast<GetDXGIInfoQueueFunc>(GetProcAddress(pDxgiDebugModule, "DXGIGetDebugInterface"));

		hr = pDXGIGetDebugInterfaceFunc(IID_PPV_ARGS(&mP_InfoQueue));

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get DXGI info queue. Error code: {}", hr);

		hr = pDXGIGetDebugInterfaceFunc(IID_PPV_ARGS(&mP_DebugInterface));

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get DXGIDebug interface. Error code: {}", hr);
			);

		InitDWrite();

		CreateViews();
		BindViews();

		SetViewport();

		m_ShaderLibrary.CreateShaderSets(mP_Device);
		m_ShaderLibrary.BindSet(SHADER_SET_TYPE_QUAD, mP_Context);

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

	void Graphics::InitDWrite() noexcept
	{
		ComPtr<IDXGISurface> pBackBufferSurface;
		HRESULT hr = mP_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBufferSurface));

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to retrieve back buffer. Error code: {}", hr);

		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, mP_FactoryD2D.GetAddressOf());

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create Direct2D factory. Error code: {}", hr);

		/* TODO: Research the distinction between DWRITE_FACTORY_TYPE_SHARED and DWRITE_FACTORY_TYPE_ISOLATED. */
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(mP_FactoryDWrite.GetAddressOf())
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create DirectWrite factory. Error code: {}", hr);

		hr = mP_FactoryDWrite->CreateTextFormat(
			L"Gabriola",                // Font family name.
			NULL,                       // Font collection (NULL sets it to use the system font collection).
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			24.0f,
			L"en-us",
			&mP_TextFormat
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create text format. Error code: {}", hr);

		hr = mP_TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to set text alignment. Error code: {}", hr);

		hr = mP_TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to set paragraph alignment. Error code: {}", hr);
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

		ComPtr<IDXGISurface> pBackBufferSurface;
		hr = pBackBuffer.As(&pBackBufferSurface);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to retrieve back buffer as IDXGISurface. Error code: {}", hr);

		hr = mP_FactoryD2D->CreateDxgiSurfaceRenderTarget(
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
			&mP_SC_Brush
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create ID2D1SolidColorBrush. Error code: {}", hr);
	}

	void Graphics::BindViews() noexcept
	{
		mP_Context->OMSetRenderTargets(1, mP_RTV.GetAddressOf(), mP_DSV.Get());
	}

	void Graphics::ReleaseViews() noexcept
	{
		mP_RTV.Reset();
		mP_DSV.Reset();
		mP_SC_Brush.Reset();
		mP_D2D_RT.Reset();
	}

	void Graphics::SetViewport() noexcept
	{
		Float2 clientArea = m_Window.ClientResolution();

		CD3D11_VIEWPORT viewport(
			0.0f, 0.0f, clientArea.x, clientArea.y
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
		 * the partially built command list as well (by calling ID3D11DeviceContext::ClearState,
		 * then ID3D11DeviceContext::FinishCommandList, then Release on the command list). */
		hr = mP_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, swapDesc.Flags);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to resize buffers.");

		CreateViews();
		BindViews();

		SetViewport();

		m_ShaderLibrary.BindSet(SHADER_SET_TYPE_QUAD, mP_Context);
	}

	void Graphics::OnResizeThunk(Float2 resolution, void* pThis) noexcept
	{
		reinterpret_cast<Graphics*>(pThis)->OnResizeCallback(resolution);
	}
}