#include "Win32/D3D/_11/Api_D3D11.hpp"
#include "Win32/PlatformOS_Win32.hpp"
#include "Common/Assert.hpp"
#include "Common/Cast.hpp"
#include "Common/Config.hpp"
#include "Common/RuntimeFailure.hpp"

#include <d3dcompiler.h>

#define API_FAILURE_V(x, fmt, ...)		  RUNTIME_FAILURE_V(x, GetActiveLogger(), fmt, __VA_ARGS__)
#define API_FAILURE_IF(x, msg)			  RUNTIME_FAILURE_IF(x, msg, GetActiveLogger())
#define API_FAILURE_IF_V(x, fmt, ...)	  RUNTIME_FAILURE_IF_V(x, GetActiveLogger(), fmt, __VA_ARGS__)

namespace Platform::Win32::Graphics::D3D::_11
{
    Api::Api(const PlatformSettings& settings, ::HWND hWnd) noexcept
    {
        Init(settings, hWnd);
    }

    Api::~Api() noexcept
    {
        Shutdown();
    }

	void Api::Clear(const float(&rgba)[4]) noexcept
	{
		mP_Context->ClearRenderTargetView(mP_RTV.Get(), rgba);
		mP_Context->ClearDepthStencilView(mP_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void Api::Present() noexcept
	{
		::HRESULT hr = mP_SwapChain->Present(m_PresentSyncInterval, m_PresentFlags);

		if (m_IsDebugLayerPresent)
		{
			if (mP_InfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL) == 0)
				return;

			LogWarning("(Api_D3D11) Debug messages generated after presenting.");
		}

		if (!FAILED(hr))
		{
			BindSwapChainViews();
			return;
		}

		hr = mP_Device->GetDeviceRemovedReason();

		API_FAILURE_IF_V(
			hr == DXGI_ERROR_DEVICE_REMOVED,
			"(Api_D3D11) Device removed. Reason: {}", 
			hr
		);

		API_FAILURE_IF_V(
			FAILED(hr),
			"(Api_D3D11) Error occured after presenting. Error: {}",
			hr
		);
	}

	void Api::Draw(
		uint32_t numVertices,
		uint32_t startVertexLocation
	) noexcept
	{
		mP_Context->Draw(
			numVertices,
			startVertexLocation
		);
	}

	void Api::DrawIndexed(
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

	void Api::DrawIndexedInstanced(
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

	void Api::CompileShader(ShaderType type, const char* pFilePath) noexcept
	{
		switch (type)
		{
		case ShaderType::Vertex:
			CompileVertexShader(pFilePath);
			break;
		case ShaderType::Pixel:
			break;
		case ShaderType::Unknown:
		default:
			break;
		}
	}

    void Api::Init(const PlatformSettings& settings, ::HWND hWnd) noexcept
    {
		API_FAILURE_IF(!hWnd, "(Api_D3D11) Provided HWND is nullptr.");

        ::DXGI_SWAP_CHAIN_DESC scDesc = {};
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
		scDesc.OutputWindow = hWnd;
		scDesc.Windowed = true;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.Flags = 0;

        /* NOTE: D3D11_CREATE_DEVICE_BGRA_SUPPORT is required for D3D11 - D2D interop. */
		::UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        /* NOTE: If D3D11_CREATE_DEVICE_DEBUG is not provided,
		 * the debug layer will not be able to be retrieved. */
		#if CONFIG_DEBUG
			flags |= D3D11_CREATE_DEVICE_DEBUG;
		#elif CONFIG_REL_DEBUG
			if (m_Config.IsGraphicsDebugging)
			{
				LogInfo(
                    "(Api_D3D11) Creating device with debug layer in non-debug build "
                    "config since a graphics debugger is active.");
				flags |= D3D11_CREATE_DEVICE_DEBUG;
			}
		#elif
			if (m_Config.IsGraphicsDebugging)
				LogWarning(
                    "(Api_D3D11) Graphics debugger is active in a distribution build. "
                    "The flag will be ignored. (no debug layer will be loaded)"
                );
		#endif

        ::HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
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

		API_FAILURE_IF_V(
			FAILED(hr),
			"(Api_D3D11) Failed to create device and swap chain. Error code: {}", 
			hr
		);

		#ifdef CONFIG_NDIST
			if (flags & D3D11_CREATE_DEVICE_DEBUG)
				RetrieveDebugLayer();
		#endif

		CreateSwapChainViews();
		BindSwapChainViews();

		::RECT clientArea = {};

		if (!GetClientRect(hWnd, &clientArea))
			LogWarning("(Api_D3D11) Failed to set viewport on initialization.");

		SetViewport(
			Cast<float>(clientArea.right),
			Cast<float>(clientArea.bottom)
		);

		mP_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void Api::Shutdown() noexcept
    {
    }
    
	void Api::CreateSwapChainViews() noexcept
	{
		ComPtr<::ID3D11Texture2D> pBackBuffer;
		::HRESULT hr = mP_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		API_FAILURE_IF_V(
			FAILED(hr),
			"(Api_D3D11) Failed to retrieve back buffer. Error code: {}",
			hr
		);

		hr = mP_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &mP_RTV);

		API_FAILURE_IF_V(
			FAILED(hr),
			"(Api_D3D11) Failed to create render target view. Error code: {}",
			hr
		);

		::D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		ComPtr<::ID3D11DepthStencilState> pDSState;
		hr = mP_Device->CreateDepthStencilState(&dsDesc, &pDSState);

		API_FAILURE_IF_V(
			FAILED(hr),
			"(Api_D3D11) Failed to create depth stencil state. Error code: {}",
			hr
		);

		mP_Context->OMSetDepthStencilState(pDSState.Get(), 1);

		::D3D11_TEXTURE2D_DESC backBufferSurfaceDesc = {};
		pBackBuffer->GetDesc(&backBufferSurfaceDesc);

		ComPtr<::ID3D11Texture2D> pDSTexture;
		::D3D11_TEXTURE2D_DESC descDepth = {};
		descDepth.Width = backBufferSurfaceDesc.Width;
		descDepth.Height = backBufferSurfaceDesc.Height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = ::DXGI_FORMAT_D32_FLOAT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = ::D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = ::D3D11_BIND_DEPTH_STENCIL;

		hr = mP_Device->CreateTexture2D(&descDepth, nullptr, &pDSTexture);

		API_FAILURE_IF_V(
			FAILED(hr),
			"(Api_D3D11) Failed to create depth stencil buffer. Error code: {}",
			hr
		);

		::D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
		descDSV.Format = ::DXGI_FORMAT_UNKNOWN;
		descDSV.ViewDimension = ::D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		hr = mP_Device->CreateDepthStencilView(pDSTexture.Get(), &descDSV, &mP_DSV);
		
		API_FAILURE_IF_V(
			FAILED(hr),
			"(Api_D3D11) Failed to create depth stencil view. Error code: {}",
			hr
		);
	}

	void Api::BindSwapChainViews() noexcept
	{
		mP_Context->OMSetRenderTargets(1, mP_RTV.GetAddressOf(), mP_DSV.Get());
	}

	void Api::ReleaseSwapChainViews() noexcept
	{
		mP_RTV.Reset();
		mP_DSV.Reset();
	}

	void Api::SetViewport(float width, float height) noexcept
	{
		CD3D11_VIEWPORT viewport(
			0.0f, 0.0f, width, height
		);

		mP_Context->RSSetViewports(1, &viewport);
	}

	void Api::CompileVertexShader(const std::filesystem::path& filePath) noexcept
	{
		if (!std::filesystem::exists(filePath))
			return;

		ComPtr<::ID3DBlob> pErrorBlob;
		ComPtr<::ID3DBlob> pShaderBlob;

		constexpr std::string_view EntryPoint = "main";
		constexpr std::string_view Target = "vs_5_0";

		::UINT flags1 = 0;
		::UINT flags2 = 0;

		flags1 |= D3DCOMPILE_WARNINGS_ARE_ERRORS;

	#if CONFIG_DEBUG
		flags1 |= D3DCOMPILE_DEBUG;
	#elif CONFIG_NDIST
		flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL2;
	#else
		flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
	#endif

		::HRESULT hr = D3DCompileFromFile(
			filePath.c_str(),
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			EntryPoint.data(),
			Target.data(),
			flags1,
			flags2,
			&pShaderBlob,
			&pErrorBlob
		);

		if (SUCCEEDED(hr))
			return;

		if (pErrorBlob)
			LogFatal(Cast<const char*>(pErrorBlob->GetBufferPointer()));

		std::string filePathStr = filePath.generic_string();

		API_FAILURE_V(
			FAILED(hr),
			"(Api_D3D11) Failed to compile shader. Error: {}, Path: {}",
			hr,
			filePathStr
		);
	}

	void Api::RetrieveDebugLayer() noexcept
	{
	#ifdef CONFIG_NDIST
		::HMODULE hDxgiDebugModule = ::GetModuleHandleW(L"Dxgidebug.dll");

		if (hDxgiDebugModule == nullptr)
		{
			LogWarning(
				"(Api_D3D11) Failed to get Dxgidebug.dll module handle. Debug info "
				" will not be available. (Graphics Debugger interference?)"
			);

			return;
		}

		LogInfo("(Api_D3D11) Successfuly loaded Dxgidebug.dll module.");

		using GetDXGIInfoQueueFunc = ::HRESULT(*)(REFIID, void**);
		GetDXGIInfoQueueFunc pDXGIGetDebugInterfaceFunc = Reinterpret<GetDXGIInfoQueueFunc>(
			::GetProcAddress(hDxgiDebugModule, "DXGIGetDebugInterface")
		);

		
		API_FAILURE_IF(
			!pDXGIGetDebugInterfaceFunc,
			"(Api_D3D11) Failed to retrieve DXGIGetDebugInterface symbol."
		);

		/* To make MSVC happy... */
		if (!pDXGIGetDebugInterfaceFunc)
			return;

		::HRESULT hr = pDXGIGetDebugInterfaceFunc(IID_PPV_ARGS(&mP_InfoQueue));

		API_FAILURE_IF_V(
			FAILED(hr),
			"(Api_D3D11) Failed to get DXGIDebug interface. Error code: {}", 
			hr
		);
		
		m_IsDebugLayerPresent = true;
	#endif
	}
}