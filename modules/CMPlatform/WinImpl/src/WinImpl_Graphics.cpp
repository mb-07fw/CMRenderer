#include "CMPlatform/WinImpl_PCH.hpp"
#include "CMPlatform/WinImpl_Graphics.hpp"
#include "CMPlatform/WinImpl_Platform.hpp"

namespace CMEngine::Platform::WinImpl
{
	extern WinImpl_Platform* gP_PlatformInstance;
	extern void WinImpl_Platform_EnforceInstantiated();

	CM_DYNAMIC_LOAD void WinImpl_Graphics_Clear(RGBANorm color)
	{
		WinImpl_Platform_EnforceInstantiated();

		gP_PlatformInstance->Impl_Graphics().Impl_Clear(color);
	}

	CM_DYNAMIC_LOAD void WinImpl_Graphics_Present()
	{
		WinImpl_Platform_EnforceInstantiated();

		gP_PlatformInstance->Impl_Graphics().Impl_Present();
	}

	CM_DYNAMIC_LOAD void WinImpl_Graphics_Draw(const void* pBuffer, const DrawDescriptor* pDescriptor)
	{
		WinImpl_Platform_EnforceInstantiated();

		gP_PlatformInstance->Impl_Graphics().Impl_Draw(pBuffer, *pDescriptor);
	}

	Graphics::Graphics(Window& window) noexcept
		: IGraphics(
		    GraphicsFuncTable(
				WinImpl_Graphics_Clear,
				WinImpl_Graphics_Present,
				WinImpl_Graphics_Draw
			)
		  ),
		  m_Window(window)
	{
		Impl_Init();
		m_Window.Impl_SetCallbackOnResize(Impl_OnResizeThunk, this);
	}

	Graphics::~Graphics() noexcept
	{
		m_Window.Impl_RemoveCallbackOnResize(Impl_OnResizeThunk, this);
		Impl_Shutdown();
	}

	void Graphics::Impl_Update() noexcept
	{
		/* TODO: Define an interface for a shader library based on WinImpl_ShaderLibrary in CMPlatform_Core.
		 *		 Move all clearing, drawing, and presenting to CMEngine side.
		 *         - Provide an abstraction for creating vertex buffers. (Worry about managing them later)
		 */
		ComPtr<ID3D11Buffer> pVBuffer;

		Float2 vertices[] = {
			{  0.0f,  0.5f },
			{  0.5f, -0.5f },
			{ -0.5f, -0.5f }
		};

		CD3D11_BUFFER_DESC vbDesc(
			sizeof(vertices),
			D3D11_BIND_VERTEX_BUFFER
			);

		D3D11_SUBRESOURCE_DATA vbSubData = {};
		vbSubData.pSysMem = vertices;

		HRESULT hr = mP_Device->CreateBuffer(&vbDesc, &vbSubData, &pVBuffer);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create vertex buffer. Erroe code: {}", hr);

		UINT stride = sizeof(Float2);
		UINT offset = 0;

		mP_Context->IASetVertexBuffers(0, 1, pVBuffer.GetAddressOf(), &stride, &offset);

		Impl_Clear(RGBANorm::Black());

		mP_Context->Draw(std::size(vertices), 0);

		Impl_Present();
	}

	void Graphics::Impl_Clear(RGBANorm color) noexcept
	{
		mP_Context->ClearRenderTargetView(mP_RTV.Get(), color.rgba);
	}

	void Graphics::Impl_Present() noexcept
	{
		HRESULT hr = mP_SwapChain->Present(m_PresentSyncInterval, m_PresentFlags);

		if (!FAILED(hr))
		{
			mP_Context->OMSetRenderTargets(1, mP_RTV.GetAddressOf(), nullptr);
			return;
		}

		if (hr == DXGI_ERROR_DEVICE_REMOVED)
			spdlog::critical("(WinImpl_Graphics) Internal error: Device removed. Reason: {}", mP_Device->GetDeviceRemovedReason());
		else
			spdlog::critical("(WinImpl_Graphics) Internal error: Errer occured after presenting. Error code: {}", hr);

		if (mP_InfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL) == 0)
			return;

		spdlog::warn("(WinImpl_Graphics) Internal warning: Debug messages generated after presenting.");
	}

	void Graphics::Impl_Draw(const void* pBuffer, const DrawDescriptor& descriptor) noexcept
	{
	}

	void Graphics::Impl_Init() noexcept
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

		UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#ifdef CM_PLATFORM_DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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
		
		m_ShaderLibrary.CreateShaderSets(mP_Device);

		Impl_CreateViews();
		Impl_BindViews();

		Impl_SetViewport();

		m_ShaderLibrary.BindSet(SHADER_SET_TYPE_QUAD, mP_Context);
	}

	void Graphics::Impl_Shutdown() noexcept
	{
	}

	void Graphics::Impl_CreateViews() noexcept
	{
		ComPtr<ID3D11Resource> pBackBuffer;
		HRESULT hr = mP_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to retrieve back buffer. Error code: {}", hr);

		hr = mP_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &mP_RTV);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create render target view. Error code: {}", hr);
	}

	void Graphics::Impl_BindViews() noexcept
	{
		mP_Context->OMSetRenderTargets(1, mP_RTV.GetAddressOf(), nullptr);
	}

	void Graphics::Impl_ReleaseViews() noexcept
	{
		mP_RTV.Reset();
		mP_DSV.Reset();
	}

	void Graphics::Impl_SetViewport() noexcept
	{
		Float2 clientArea = m_Window.Impl_ClientResolution();

		CD3D11_VIEWPORT viewport(
			0.0f, 0.0f, clientArea.x, clientArea.y
		);

		mP_Context->RSSetViewports(1, &viewport);

		mP_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void Graphics::Impl_OnResizeCallback(Float2 resolution) noexcept
	{
		mP_Context->ClearState();
		mP_Context->Flush();

		Impl_ReleaseViews();

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

		Impl_CreateViews();
		Impl_BindViews();

		Impl_SetViewport();

		m_ShaderLibrary.BindSet(SHADER_SET_TYPE_QUAD, mP_Context);
	}

	void Graphics::Impl_OnResizeThunk(Float2 resolution, void* pThis) noexcept
	{
		reinterpret_cast<Graphics*>(pThis)->Impl_OnResizeCallback(resolution);
	}
}