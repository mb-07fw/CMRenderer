#include "CMPlatform/WinImpl_PCH.hpp"
#include "CMPlatform/WinImpl_Graphics.hpp"
#include "CMPlatform/WinImpl_Platform.hpp"

namespace CMEngine::Platform::WinImpl
{
#pragma region ShaderLibrary
	void BasicShaderSet::CreateBasicShaders(Microsoft::WRL::ComPtr<ID3D11Device> pDevice) noexcept
	{
		HRESULT hResult = pDevice->CreateVertexShader(
			VertexData.pBytecode->GetBufferPointer(),
			VertexData.pBytecode->GetBufferSize(),
			nullptr,
			pVertexShader.GetAddressOf()
		);

		if (FAILED(hResult))
			spdlog::critical("(WinImpl_BasicShaderSet) Internal error: Failed to create vertex shader. Error code: `{}`",
				hResult
			);

		hResult = pDevice->CreatePixelShader(
			PixelData.pBytecode->GetBufferPointer(),
			PixelData.pBytecode->GetBufferSize(),
			nullptr,
			pPixelShader.GetAddressOf()
		);

		if (FAILED(hResult))
			spdlog::critical("(WinImpl_BasicShaderSet) Internal error: Failed to create pixel shader. Error code: `{}`",
				hResult
			);
	}

	void ShaderSetQuad::CreateShaders(Microsoft::WRL::ComPtr<ID3D11Device> pDevice) noexcept
	{
		CreateBasicShaders(pDevice);
	}

	ShaderLibrary::ShaderLibrary() noexcept
	{
		Init();
	}

	ShaderLibrary::~ShaderLibrary() noexcept
	{
		Shutdown();
	}

	void ShaderLibrary::CreateResources(Microsoft::WRL::ComPtr<ID3D11Device> pDevice) noexcept
	{
		for (const std::shared_ptr<BasicShaderSet>& pSet : m_Sets)
			pSet->CreateShaders(pDevice);
	}

	void ShaderLibrary::Init() noexcept
	{
		LoadAll();
	}

	void ShaderLibrary::Shutdown() noexcept
	{
	}

	void ShaderLibrary::LoadAll() noexcept
	{
		std::vector<ShaderData> data;

		LoadShaders(data);
		CreateShaderSets(data);
	}

	void ShaderLibrary::LoadShaders(std::vector<ShaderData>& outData) noexcept
	{
		std::filesystem::path compiledShaderDirectory(CM_PLATFORM_CORE_PATH_COMPILED_SHADER_DIRECTORY);

		if (!std::filesystem::exists(compiledShaderDirectory))
			spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Compiled shader directory doesn't exist: `{}`", 
				CM_PLATFORM_CORE_PATH_COMPILED_SHADER_DIRECTORY
			);
		else if (!std::filesystem::is_directory(compiledShaderDirectory))
			spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Compiled shader directory isn't a directory: `{}`",
				CM_PLATFORM_CORE_PATH_COMPILED_SHADER_DIRECTORY
			);

		for (const auto& entry : std::filesystem::directory_iterator(compiledShaderDirectory))
		{
			const std::filesystem::path& entryPath = entry.path();
			std::string entryStr = entryPath.generic_string();

			if (entryPath.extension() != ".cso")
			{
				spdlog::warn(
					"(WinImpl_ShaderLibrary) Internal warning: A file with an extension other than .cso is in compiled "
					"shader directory. Skipping file: `{}`",
					entryStr
				);

				continue;
			}

			const std::wstring& entryWStr = entryPath.native();

			std::filesystem::path fileNamePath = entryPath.filename();

			const std::wstring& fileNameWStr = fileNamePath.native();
			std::string fileNameStr = fileNamePath.generic_string();

			auto it = m_ShaderNameMap.find(fileNameWStr);

			if (it == m_ShaderNameMap.end())
				spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Failed to find a match for compiled shader name: `{}`",
					fileNameStr
				);

			ActiveShaderEnum::Enum activeShaderType = it->second;
			ShaderSetEnum::Enum correspondingSetType = ActiveToCorrespondingType(activeShaderType);

			ShaderEnum::Enum shaderType = ShaderEnum::INVALID;

			constexpr size_t FlagOffsetFromExtension = 6;

			size_t entryLength = entryWStr.length();
			size_t flagOffset = entryLength - FlagOffsetFromExtension;

			std::wstring_view flagWStr(entryWStr.data() + flagOffset, 2);
			std::string_view flagStr(entryStr.data() + flagOffset, 2);

			if (flagWStr == S_VERTEX_SHADER_FLAG)
				shaderType = ShaderEnum::VERTEX;
			else if (flagWStr == S_PIXEL_SHADER_FLAG)
				shaderType = ShaderEnum::PIXEL;
			else
				spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Compiled shader has invalid flag: `{}`. Compiled shader: `{}`",
					flagStr,
					entryStr
				);

			Microsoft::WRL::ComPtr<ID3DBlob> pShaderBytecode;
			HRESULT hResult = D3DReadFileToBlob(entryWStr.data(), pShaderBytecode.GetAddressOf());

			if (FAILED(hResult))
				spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Failed to read compiled shader bytecode. Error code: `{}` Shader: `{}`",
					hResult,
					entryStr
				);
			else if (pShaderBytecode == nullptr)
				spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Read bytecode of shader is nullptr: `{}`",
					entryStr
				);
			
			outData.emplace_back(shaderType, activeShaderType, correspondingSetType, fileNameWStr, pShaderBytecode);
		}
	}

	void ShaderLibrary::CreateShaderSets(const std::vector<ShaderData>& data) noexcept
	{
		for (ShaderSetEnum::Enum setType : G_IMPLEMENTED_SHADER_SETS)
		{
			const ShaderData* pVertexData = nullptr;
			const ShaderData* pPixelData = nullptr;

			for (const ShaderData& currentData : data)
			{
				if (setType != currentData.CorrespondingSet)
					continue;

				/* To convert name to utf-8. */
				std::filesystem::path namePath(currentData.Name);

				std::string nameStr = namePath.generic_string();

				switch (currentData.Type)
				{
				case ShaderEnum::VERTEX:
					if (pVertexData != nullptr)
						spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Found duplicate vertex-type data for the same ShaderSetType. Set: `{}`, Name: `{}`",
							static_cast<size_t>(currentData.CorrespondingSet),
							nameStr
						);

					pVertexData = &currentData;
					break;
				case ShaderEnum::PIXEL:
					if (pPixelData != nullptr)
						spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Found duplicate pixel-type data for the same ShaderSetType. Set: `{}`, Name: `{}`",
							static_cast<size_t>(currentData.CorrespondingSet),
							nameStr
						);

					pPixelData = &currentData;
					break;
				case ShaderEnum::INVALID: [[fallthrough]];
				default:
					spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Data has an invalid ShaderType. Set: `{}` Name: `{}`",
						static_cast<size_t>(currentData.CorrespondingSet),
						nameStr
					);
				}
			}

			if (pVertexData == nullptr)
			{
				spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Failed to find a vertex-type ShaderData for the current implement ShaderSetType. Set: `{}`",
					static_cast<size_t>(setType)
				);

				std::exit(-1);
			}
			else if (pPixelData == nullptr)
			{
				spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Failed to find a pixel-type ShaderData for the current implement ShaderSetType. Set: `{}`",
					static_cast<size_t>(setType)
				);

				std::exit(-1);
			}

			switch (setType)
			{
			case ShaderSetEnum::QUAD:
				m_Sets.emplace_back(std::make_shared<ShaderSetQuad>(*pVertexData, *pPixelData));
				break;
			case ShaderSetEnum::INVALID: [[fallthrough]];
			default:
				spdlog::critical("(WinImpl_ShaderLibrary) Internal error: Failed to construct a shader set for the current ShaderSetType. Set: `{}`",
					static_cast<size_t>(setType)
				);
			}
		}
	}
#pragma endregion

	extern WinImpl_Platform* gP_PlatformInstance;
	extern void WinImpl_Platform_EnforceInstantiated();

	CM_DYNAMIC_LOAD void WinImpl_Graphics_Clear(ColorNorm color)
	{
		WinImpl_Platform_EnforceInstantiated();

		gP_PlatformInstance->Impl_Graphics().Impl_Clear(color);
	}

	CM_DYNAMIC_LOAD void WinImpl_Graphics_Present()
	{
		WinImpl_Platform_EnforceInstantiated();

		gP_PlatformInstance->Impl_Graphics().Impl_Present();
	}

	Graphics::Graphics(Window& window) noexcept
		: IGraphics(
			GraphicsFuncTable(
				WinImpl_Graphics_Clear,
				WinImpl_Graphics_Present
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
		ColorNorm rgba = { 0.0f, 0.0f, 0.0f, 0.0f };

		Impl_Clear(rgba);
		Impl_Present();
	}

	void Graphics::Impl_Clear(ColorNorm color) noexcept
	{
		mP_Context->ClearRenderTargetView(mP_RTV.Get(), color.rgba);
		mP_Context->ClearDepthStencilView(mP_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void Graphics::Impl_Present() noexcept
	{
		mP_SwapChain->Present(m_PresentSyncInterval, m_PresentFlags);

		Impl_BindViews();
	}

	void Graphics::Impl_Init() noexcept
	{
		Impl_InitPipeline();

		m_Library.CreateResources(mP_Device);

		spdlog::info("(WinImpl_Graphics) Graphics Init!");
	}

	void Graphics::Impl_Shutdown() noexcept
	{
		spdlog::info("(WinImpl_Graphics) Graphics Shutdown!");
	}

	void Graphics::Impl_InitPipeline() noexcept
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
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create device. Error code: {}", hResult);
			std::exit(-1);
		}
		else if (succeededLevel < D3D_FEATURE_LEVEL_11_0)
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to match D3D11_1 feature level. Level: {}", static_cast<long>(succeededLevel));
			std::exit(-1);
		}

		hResult = mP_Device->QueryInterface(IID_PPV_ARGS(mP_InfoQueue.GetAddressOf()));

		if (FAILED(hResult))
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get DXGI info queue. Error code: {}", hResult);
			std::exit(-1);
		}

		Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice = nullptr;
		hResult = mP_Device.As(&pDXGIDevice);

		if (FAILED(hResult))
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get DXGI interface from device. Error code: {}", hResult);
			std::exit(-1);
		}

		Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter = nullptr;
		hResult = pDXGIDevice->GetAdapter(&pDXGIAdapter);

		if (FAILED(hResult))
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get DXGI adapter interface from DXGI device. Error code: {}", hResult);
			std::exit(-1);
		}

		hResult = pDXGIAdapter->GetParent(IID_PPV_ARGS(&mP_Factory));

		if (FAILED(hResult))
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get IDXGIFactory2 interface from DXGI adapter interface. Error code: {}", hResult);
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
			m_Window.Impl_HWND(),
			&desc,
			&fullscreenDesc,
			nullptr, /* Don't restrict output to any adapters. */
			mP_SwapChain.GetAddressOf()
		);

		if (FAILED(hResult))
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create swap chain for HWND. Error code: {}", hResult);
			std::exit(-1);
		}

		Impl_CreateViews();
	}

	void Graphics::Impl_CreateViews() noexcept
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		HRESULT hResult = mP_SwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));

		if (FAILED(hResult))
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to retrieve back buffer. Error code: {}", hResult);
			std::exit(-1);
		}

		hResult = mP_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, mP_RTV.GetAddressOf());

		if (FAILED(hResult))
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create render target view. Error code: {}", hResult);
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
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create depth stencil texture. Error code: {}", hResult);
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
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create depth stencil state. Error code: {}", hResult);
			std::exit(-1);
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0u;

		hResult = mP_Device->CreateDepthStencilView(pDSTexture.Get(), &dsvDesc, mP_DSV.GetAddressOf());

		if (FAILED(hResult))
		{
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to create depth stencil view. Error code: {}", hResult);
			std::exit(-1);
		}

		/* TODO: Move viewport and topology somewhere else when relevant. */
		CD3D11_VIEWPORT viewport(
			0.0f,
			0.0f,
			static_cast<float>(backBufferDesc.Width),
			static_cast<float>(backBufferDesc.Height)
		);

		mP_Context->RSSetViewports(1, &viewport);

		mP_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void Graphics::Impl_ReleaseViews() noexcept
	{
		mP_RTV.Reset();
		mP_DSV.Reset();
	}

	void Graphics::Impl_BindViews() noexcept
	{
		mP_Context->OMSetRenderTargets(1u, mP_RTV.GetAddressOf(), mP_DSV.Get());
	}

	void Graphics::Impl_OnResizeCallback(ScreenResolution resolution) noexcept
	{
		/* Before resizing the swap chain, all views on the swap chain
		 * and their references should be released. 
		 * 
		 * Future note for deferred rendering:
		 *   
		 *   If a view is bound to a deferred context, the partially built
		 *     command list must be discarded as well by calling
		 *     ID3D11Device::FinishCommandList after ClearState (and releasing it).
		 * 
		 * (https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers) */
		mP_Context->ClearState();

		Impl_ReleaseViews();

		DXGI_SWAP_CHAIN_DESC scDesc;
		HRESULT hResult = mP_SwapChain->GetDesc(&scDesc);

		if (FAILED(hResult))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to get swap chain's descriptor.");

		hResult = mP_SwapChain->ResizeBuffers(
			0, /* Preserve the original amount of buffers. */
			static_cast<UINT>(resolution.Res.x),
			static_cast<UINT>(resolution.Res.y),
			DXGI_FORMAT_UNKNOWN, /* Preserve original format. */
			scDesc.Flags
		);

		if (FAILED(hResult))
			spdlog::critical("(WinImpl_Graphics) Internal error: Failed to resize swap chain.");

		Impl_CreateViews();
		Impl_BindViews();
	}

	void Graphics::Impl_OnResizeThunk(ScreenResolution resolution, void* pThis) noexcept
	{
		Graphics* pGraphics = reinterpret_cast<Graphics*>(pThis);
		pGraphics->Impl_OnResizeCallback(resolution);
	}
}