#include "CMPlatform/WinImpl_PCH.hpp"
#include "CMPlatform/WinImpl_Graphics.hpp"
#include "CMPlatform/WinImpl_Platform.hpp"

namespace CMEngine::Platform::WinImpl
{
	ShaderLibrary::ShaderLibrary() noexcept
	{
		Init();
	}

	ShaderLibrary::~ShaderLibrary() noexcept
	{
		Shutdown();
	}

	void ShaderLibrary::Init() noexcept
	{
	}

	void ShaderLibrary::Shutdown() noexcept
	{
	}

	extern WinImpl_Platform* gP_PlatformInstance;

	extern void WinImpl_EnforceInstantiated();

	Graphics::Graphics(HWND hWnd) noexcept
		: mP_HWND(hWnd)
	{
		Impl_Init();
	}

	Graphics::~Graphics() noexcept
	{
		Impl_Shutdown();
	}

	void Graphics::Impl_Update() noexcept
	{
		float rgba[] = { 0.0f, 0.0f, 0.0f, 0.0f };

		Impl_BindViews();

		mP_Context->ClearRenderTargetView(mP_RTV.Get(), rgba);
		mP_Context->ClearDepthStencilView(mP_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		mP_SwapChain->Present(m_PresentSyncInterval, m_PresentFlags);
	}

	void Graphics::Impl_Init() noexcept
	{
		D3D_FEATURE_LEVEL succeededLevel;
		HRESULT hResult = S_OK;

		hResult = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_SINGLETHREADED,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			mP_Device.GetAddressOf(),
			&succeededLevel,
			mP_Context.GetAddressOf()
		);

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to create device. Error code: {}", hResult);
			std::exit(-1);
		}
		else if (succeededLevel < D3D_FEATURE_LEVEL_11_0)
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to match D3D11_1 feature level. Level: {}", static_cast<long>(succeededLevel));
			std::exit(-1);
		}

		hResult = mP_Device->QueryInterface(IID_PPV_ARGS(mP_InfoQueue.GetAddressOf()));
		//hResult = mP_Device.As(&mP_InfoQueue);

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to get DXGI info queue. Error code: {}", hResult);
			std::exit(-1);
		}

		Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice = nullptr;
		hResult = mP_Device.As(&pDXGIDevice);

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to get DXGI interface from device. Error code: {}", hResult);
			std::exit(-1);
		}

		Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter = nullptr;
		hResult = pDXGIDevice->GetAdapter(&pDXGIAdapter);

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to get DXGI adapter interface from DXGI device. Error code: {}", hResult);
			std::exit(-1);
		}

		hResult = pDXGIAdapter->GetParent(IID_PPV_ARGS(&mP_Factory));

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to get IDXGIFactory2 interface from DXGI adapter interface. Error code: {}", hResult);
			std::exit(-1);
		}

		BOOL allowTearing = FALSE;
		mP_Factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

		UINT swapChainFlags = 0;
		if (allowTearing)
		{
			swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

			m_PresentSyncInterval = S_PRESENT_SYNC_INTERVAL_TEARING;
			m_PresentFlags = DXGI_PRESENT_ALLOW_TEARING;
		}

		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Width = 0;
		desc.Height = 0;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Stereo = false; /* Don't use steroscopic. */
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		desc.BufferCount = 2;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; /* Use over standard _DISCARD for higher performance and compatibility. */
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; /* Unspecified as the window itself needs to be opaque for CreateSwapChainForHwnd. */
		desc.Flags = swapChainFlags;

		/* Here so I don't forget this later:
		 * (from https://learn.microsoft.com/en-us/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_chain_flag)
		 * DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH:
		 *   Set this flag to enable an application to switch modes by calling IDXGISwapChain::ResizeTarget.
	     *   When switching from windowed to full-screen mode, the display mode (or monitor resolution) will
		 *     be changed to match the dimensions of the application window.
		 *
		 * DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING:
		 *   Tearing support is a requirement to enable displays that support variable refresh rates to function
		 *   properly when the application presents a swap chain tied to a full screen borderless window. Win32
		 *   apps can already achieve tearing in fullscreen exclusive mode by calling SetFullscreenState(TRUE),
		 *     but the recommended approach for Win32 developers is to use this tearing flag instead. This flag
		 *   requires the use of a DXGI_SWAP_EFFECT_FLIP_* swap effect.
		 *   To check for hardware support of this feature, refer to IDXGIFactory5::CheckFeatureSupport. For
		 *     usage information refer to IDXGISwapChain::Present and the DXGI_PRESENT flags.
		 */

		/* NOTE:
		 * (from https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_fullscreen_desc)
		 * Setting the numerator to 0 forces the native display's refresh rate. */
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
		fullscreenDesc.RefreshRate.Numerator = 0;
		fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fullscreenDesc.Windowed = true;

		/* Important note:
		 * 
		 * from (https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgifactory2-createswapchainforhwnd)
		 *   Because you can associate only one flip presentation model swap chain at a time with an HWND,
		 *     the Microsoft Direct3D 11 policy of deferring the destruction of objects can cause problems
		 *     if you attempt to destroy a flip presentation model swap chain and replace it with another
		 *     swap chain. For more info about this situation, see Deferred Destruction Issues with Flip
		 *     Presentation Swap Chains.
		 */
		hResult = mP_Factory->CreateSwapChainForHwnd(
			mP_Device.Get(),
			mP_HWND,
			&desc,
			&fullscreenDesc,
			nullptr, /* Don't restrict output to any adapters. */
			mP_SwapChain.GetAddressOf()
		);

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to create swap chain for HWND. Error code: {}", hResult);
			std::exit(-1);
		}

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		hResult = mP_SwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to retrieve back buffer. Error code: {}", hResult);
			std::exit(-1);
		}

		hResult = mP_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, mP_RTV.GetAddressOf());

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to create render target view. Error code: {}", hResult);
			std::exit(-1);
		}

		D3D11_TEXTURE2D_DESC backBufferDesc;
		pBackBuffer->GetDesc(&backBufferDesc);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDSTexture;
		D3D11_TEXTURE2D_DESC dsTextureDesc = {};
		dsTextureDesc.Width = backBufferDesc.Width;
		dsTextureDesc.Height = backBufferDesc.Height;
		dsTextureDesc.MipLevels = 1;
		dsTextureDesc.ArraySize = 1;
		dsTextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;	
		dsTextureDesc.SampleDesc.Count = 1;
		dsTextureDesc.SampleDesc.Quality = 0;
		dsTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		dsTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hResult = mP_Device->CreateTexture2D(&dsTextureDesc, nullptr, pDSTexture.GetAddressOf());

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to create depth stencil texture. Error code: {}", hResult);
			std::exit(-1);
		}

		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable = false;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
		hResult = mP_Device->CreateDepthStencilState(&dsDesc, pDSState.GetAddressOf());

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to create depth stencil state. Error code: {}", hResult);
			std::exit(-1);
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0u;

		hResult = mP_Device->CreateDepthStencilView(pDSTexture.Get(), &dsvDesc, mP_DSV.GetAddressOf());

		if (FAILED(hResult))
		{
			spdlog::error("(WinImpl_Graphics) Internal error: Failed to create depth stencil view. Error code: {}", hResult);
			std::exit(-1);
		}

		CD3D11_VIEWPORT viewport(0.0f, 0.0f, 800, 600);
		mP_Context->RSSetViewports(1, &viewport);

		mP_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		spdlog::info("(WinImpl_Graphics) Graphics Init!");
	}

	void Graphics::Impl_Shutdown() noexcept
	{
		spdlog::info("(WinImpl_Graphics) Graphics Shutdown!");
	}

	void Graphics::Impl_BindViews() noexcept
	{
		mP_Context->OMSetRenderTargets(1u, mP_RTV.GetAddressOf(), mP_DSV.Get());
	}
}