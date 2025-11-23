#include "PCH.hpp"
#include "Platform/WinImpl/Platform_WinImpl.hpp"
#include "Platform/WinImpl/Graphics_WinImpl.hpp"
#include "Platform/WinImpl/Texture_WinImpl.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"
#include "Platform/WinImpl/GPUBuffer_WinImpl.hpp"
#include "Platform/WinImpl/InputLayout_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
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

	void Graphics::Clear(const Color4& color) noexcept
	{
		mP_Context->ClearRenderTargetView(mP_RTV.Get(), color.rgba);
		mP_Context->ClearDepthStencilView(mP_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();
	}

	void Graphics::Present() noexcept
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

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

	void Graphics::Draw(
		uint32_t numVertices,
		uint32_t startVertexLocation
	) noexcept
	{
		mP_Context->Draw(
			numVertices,
			startVertexLocation
		);
	}

	void Graphics::DrawIndexed(
		uint32_t numIndices,
		uint32_t startIndexLocation, 
		int32_t baseVertexLocation
	) noexcept
	{
		mP_Context->DrawIndexed(
			numIndices,
			startIndexLocation,
			baseVertexLocation
		);
	}

	void Graphics::DrawIndexedInstanced(
		uint32_t indicesPerInstance,
		uint32_t totalInstances,
		uint32_t startIndexLocation,
		int32_t baseVertexLocation,
		uint32_t startInstanceLocation
	) noexcept
	{
		mP_Context->DrawIndexedInstanced(
			indicesPerInstance,
			totalInstances,
			startIndexLocation,
			baseVertexLocation,
			startInstanceLocation
		);
	}

	[[nodiscard]] Resource<IInputLayout> Graphics::CreateInputLayout(
		std::span<const InputElement> elems,
		ShaderID vertexID
	) noexcept
	{
		Resource<InputLayout> inputLayout = std::make_unique<InputLayout>(elems);

		const ShaderData* pShaderData = m_ShaderRegistry.Retrieve(vertexID);

		if (!pShaderData)
		{
			spdlog::critical(
				"(WinImpl_Graphics) Internal error: Failed to retrieve a valid ShaderData from the provided ShaderID. "
				"Index: {}, ShaderType: {}, AssignedType: {}",
				vertexID.Index,
				(uint16_t)vertexID.Type,
				(uint16_t)vertexID.AssignedType
			);

			return Resource<InputLayout>(nullptr);
		}
			
		inputLayout->Create(pShaderData->pBytecode, mP_Device);
		return inputLayout;
	}

	void Graphics::BindInputLayout(const Resource<IInputLayout>& inputLayout) noexcept
	{
		InputLayout* pDerived = dynamic_cast<InputLayout*>(inputLayout.get());

		if (!pDerived)
		{
			spdlog::warn(
				"(WinImpl_Graphics) [BindInputLayout] Internal warning: Attempted to bind an "
				"instance that was either nullptr or not of Platform::WinImpl::InputLayout. "
				"(how tf did you do that?)"
			);
			return;
		}

		pDerived->Upload(mP_Context);
	}

	void Graphics::DumpInputLayout(const Resource<IInputLayout>& inputLayout) noexcept
	{
		InputLayout* pDerived = dynamic_cast<InputLayout*>(inputLayout.get());

		if (!pDerived)
		{
			spdlog::warn(
				"(WinImpl_Graphics) [DumpInputLayout] Internal warning: Attempted to utilize an "
				"instance that was either nullptr or not of Platform::WinImpl::InputLayout. "
				"(how tf did you do that?)"
			);
			return;
		}

		const auto& elements =  pDerived->Elements();
		const auto& nativeElements =  pDerived->NativeElements();
		
		std::cout << '\n';
		spdlog::info("(WinImpl_Graphics) [DumpInputLayout] Elements -------");

		for (size_t i = 0; i < elements.size(); ++i)
		{
			const auto& e = elements[i];

			spdlog::info("Element: {}", i);
			spdlog::info("\tName: {}", e.Name);
			spdlog::info("\tIndex: {}", e.Index);
			spdlog::info("\tFormat: {}", DataFormatToString(e.Format));
			spdlog::info("\tInputSlot: {}", e.InputSlot);
			spdlog::info("\tAllignedByteOffset: {}", e.AlignedByteOffset);
			spdlog::info("\tInputClass: {}", InputClassToString(e.InputClass));
			spdlog::info("\tInstanceStepRate: {}", e.InstanceStepRate);
		}

		spdlog::info("(WinImpl_Graphics) [DumpInputLayout] NativeElements -------");
		for (size_t i = 0; i < nativeElements.size(); ++i)
		{
			const auto& e = nativeElements[i];

			spdlog::info("NativeElement: {}", i);
			spdlog::info("\tSemanticName: {}", e.SemanticName);
			spdlog::info("\tSemanticIndex: {}", e.SemanticIndex);
			spdlog::info("\tFormat: {}", (size_t)e.Format);
			spdlog::info("\tInputSlot: {}", e.InputSlot);
			spdlog::info("\tAllignedByteOffset: {}", e.AlignedByteOffset);
			spdlog::info("\tInputSlotClass: {}", (size_t)e.InputSlotClass);
			spdlog::info("\tInstanceDataStepRate: {}", e.InstanceDataStepRate);
		}

		std::cout << '\n';
	}

	[[nodiscard]] Resource<ITexture> Graphics::CreateTexture(std::span<std::byte> data) noexcept
	{
		Resource<Texture> texture = std::make_unique<Texture>();

		texture->Create(data, mP_Device);
		return texture;
	}

	void Graphics::BindTexture(
		const Resource<ITexture>& texture
	) noexcept
	{
		Texture* pTexture = dynamic_cast<Texture*>(texture.get());

		if (!pTexture)
		{
			spdlog::warn(
				"(WinImpl_Graphics) [BindTexture] Internal warning: Attempted to bind a texture "
				"that was either nullptr, or not of type derived from ITexture or IDXUploadable."
			);

			return;
		}

		pTexture->Upload(mP_Context);
	}

	[[nodiscard]] Resource<IBuffer> Graphics::CreateBuffer(GPUBufferType type, GPUBufferFlag flags) noexcept
	{
		switch (type)
		{
		case GPUBufferType::Invalid: [[fallthrough]];
		default:
			return Resource<IBuffer>(nullptr);
		case GPUBufferType::Vertex:
			return Resource<VertexBuffer>(new VertexBuffer(flags));
		case GPUBufferType::Index:
			return Resource<IndexBuffer>(new IndexBuffer(flags));
		case GPUBufferType::Constant:
			return Resource<ConstantBuffer>(new ConstantBuffer(flags));
		}
	}

	void Graphics::SetBuffer(const Resource<IBuffer>& buffer, const void* pData, size_t numBytes) noexcept
	{
		IGPUBuffer* pDerived = dynamic_cast<IGPUBuffer*>(buffer.get());

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

	void Graphics::BindVertexBuffer(const Resource<IBuffer>& buffer, uint32_t strideBytes, uint32_t offsetBytes, uint32_t slot) noexcept
	{
		VertexBuffer* pDerivedVB = dynamic_cast<VertexBuffer*>(buffer.get());

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

	void Graphics::BindIndexBuffer(const Resource<IBuffer>& buffer, DataFormat indexFormat, uint32_t startIndex) noexcept
	{
		IndexBuffer* pDerivedIB = dynamic_cast<IndexBuffer*>(buffer.get());

		if (!pDerivedIB)
		{
			spdlog::warn("(WinImpl_Graphics) [BindConstantBufferVS] Internal warning: Attempted to bind a buffer instance that was either nullptr, or not of type IndexBuffer.");
			return;
		}

		pDerivedIB->SetFormat(DataToDXGI(indexFormat));
		pDerivedIB->SetOffset(startIndex);
		pDerivedIB->Upload(mP_Context);
	}

	[[nodiscard]] void Graphics::BindConstantBufferVS(const Resource<IBuffer>& buffer, uint32_t slot) noexcept
	{
		ConstantBuffer* pDerivedCB = dynamic_cast<ConstantBuffer*>(buffer.get());

		if (!pDerivedCB)
		{
			spdlog::warn("(WinImpl_Graphics) [BindConstantBufferVS] Internal warning: Attempted to bind a buffer instance that was either nullptr, or not of type ConstantBuffer.");
			return;
		}

		pDerivedCB->SetType(ConstantBufferType::VS);
		pDerivedCB->SetRegister(slot);
		pDerivedCB->Upload(mP_Context);
	}

	[[nodiscard]] void Graphics::BindConstantBufferPS(const Resource<IBuffer>& buffer, uint32_t slot) noexcept
	{
		ConstantBuffer* pDerivedCB = dynamic_cast<ConstantBuffer*>(buffer.get());

		if (!pDerivedCB)
		{
			spdlog::warn("(WinImpl_Graphics) [BindConstantBufferPS] Internal warning: Attempted to bind a buffer instance that was either nullptr, or not of type ConstantBuffer.");
			return;
		}

		pDerivedCB->SetType(ConstantBufferType::PS);
		pDerivedCB->SetRegister(slot);
		pDerivedCB->Upload(mP_Context);
	}

	[[nodiscard]] ShaderID Graphics::GetShader(std::wstring_view shaderName) noexcept
	{
		return m_ShaderRegistry.QueryID(shaderName.data());
	}

	void Graphics::BindShader(ShaderID id) noexcept
	{
		m_ShaderRegistry.BindShader(id, mP_Context);
	}

	[[nodiscard]] ShaderID Graphics::LastVS() const noexcept
	{
		return m_ShaderRegistry.LastVS();
	}

	[[nodiscard]] ShaderID Graphics::LastPS() const noexcept
	{
		return m_ShaderRegistry.LastPS();
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
				spdlog::info("(WinImpl_Graphics) Internal info: Successfuly loaded Dxgidebug.dll module.");

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

		Float2 res(
			static_cast<float>(scDesc.BufferDesc.Width),
			static_cast<float>(scDesc.BufferDesc.Height)
		);

		SetViewport(res);

		m_ShaderRegistry.CreateShaders(mP_Device);

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

	void Graphics::SetViewport(Float2 res) noexcept
	{
		CD3D11_VIEWPORT viewport(
			0.0f, 0.0f, res.x, res.y
		);

		mP_Context->RSSetViewports(1, &viewport);

		mP_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void Graphics::OnResizeCallback(Float2 res) noexcept
	{
		mP_Context->ClearState();
		mP_Context->Flush();

		m_ShaderRegistry.ClearBound();

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

		SetViewport(res);
	}

	void Graphics::OnResizeThunk(Float2 res, void* pThis) noexcept
	{
		reinterpret_cast<Graphics*>(pThis)->OnResizeCallback(res);
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

	void Graphics::D2DDrawText(
		const std::wstring_view& text,
		const Float2& pos,
		const Float2& res,
		const Color4& color
	) noexcept
	{
		if (IsGraphicsDebugging())
			return;

		mP_D2D_SC_Brush->SetColor(ToD2D1ColorF(color));

		D2D1_RECT_F layoutRect = ToD2D1RectF(pos, res);

		mP_D2D_RT->DrawText(
			text.data(),
			static_cast<UINT32>(text.length()),
			mP_DW_TextFormat.Get(),
			layoutRect,
			mP_D2D_SC_Brush.Get()
		);
	}

	void Graphics::D2DDrawRect(const Float2& pos, const Float2& res, const Color4& color) noexcept
	{
		if (IsGraphicsDebugging())
			return;

		mP_D2D_SC_Brush->SetColor(ToD2D1ColorF(color));

		D2D1_RECT_F layoutRect = ToD2D1RectF(pos, res);

		mP_D2D_RT->DrawRectangle(layoutRect, mP_D2D_SC_Brush.Get());
	}
}