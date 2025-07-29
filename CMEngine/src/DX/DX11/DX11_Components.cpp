#include "Core/PCH.hpp"
#include "DX/DX11/DX11_Components.hpp"
#include "DX/DX11/DX11_Utility.hpp"
#include "Win/Win_Utility.hpp"

namespace CMEngine::DX::DX11
{
#pragma region Device
	Device::Device(Common::LoggerWide& logger) noexcept
		: m_Logger(logger)
	{
		m_Logger.LogInfoNL(L"Device [()] | Constructed.");
	}

	Device::~Device() noexcept
	{
		if (m_Created)
			Release();

		m_Logger.LogInfoNL(L"Device [~()] | Destroyed.");
	}

	void Device::Create() noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Created,
			L"Device [Create] | Attempted to create after the device has already been created."
		);

		CreateDevice();

		m_Logger.LogInfoNL(L"Device [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void Device::Release() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Created,
			L"Device [Release] | Attempted to release before creation has occured."
		);

		m_Logger.LogFatalNLIf(
			m_Released,
			L"Device [Release] | Attempted to release after Device was released previously."
		);

		// Apparently this is the best practice in order to avoid dangling references and what not,
		// I really don't know anymore...
		mP_Context->OMSetRenderTargets(0, nullptr, nullptr);
		mP_Context->ClearState();
		mP_Context->Flush();

		mP_Context.Reset();
		mP_Device.Reset();

		m_Logger.LogInfoNL(L"Device [Release] | Released.");

		m_Created = false;
		m_Released = true;
	}

	ID3D11Device1* Device::operator->() noexcept
	{
		return GetRaw();
	}

	void Device::CreateDevice() noexcept
	{
		D3D11_CREATE_DEVICE_FLAG flags = static_cast<D3D11_CREATE_DEVICE_FLAG>(D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT);
		//D3D11_CREATE_DEVICE_FLAG flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		// flags | D3D11_CREATE_DEVICE_DEBUGGABLE // For complex debugging... 
		// more stuff here : https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_create_device_flag
		CM_IF_DEBUG(flags = static_cast<D3D11_CREATE_DEVICE_FLAG>(flags | D3D11_CREATE_DEVICE_DEBUG));

		Microsoft::WRL::ComPtr<ID3D11Device> pBaseDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pBaseContext;

		D3D_FEATURE_LEVEL succeededLevel;
		HRESULT hResult = S_OK;
		hResult = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&pBaseDevice,
			&succeededLevel,
			&pBaseContext
		);

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"Device [CreateDevice] | Failed to create device : ",
			Win::Utility::TranslateError(hResult)
		);

		m_Logger.LogFatalNLVariadicIf(
			succeededLevel < D3D_FEATURE_LEVEL_11_0,
			L"Device [CreateDevice] | DirectX Feature Level was less than 11.0 (",
			Utility::D3DFeatureLevelToWStrView(succeededLevel).data(),
			L"), which is less than required for this program. Continuation will only result in errors."
		);

		m_Logger.LogInfoNLAppend(
			L"Device [CreateDevice] | Feature level in use : ",
			Utility::D3DFeatureLevelToWStrView(succeededLevel)
		);

		// Upgrade device to ID3D11Device1. (D3D11.1)
		hResult = pBaseDevice.As(&mP_Device);

		m_Logger.LogFatalNLIf(FAILED(hResult), L"Device [CreateDevice] | Failed to upgrade device to ID3D11Device1.");

		// Upgrade device context to ID3D11DeviceContext1. (D3D11.1)
		hResult = pBaseContext.As(&mP_Context);

		m_Logger.LogFatalNLIf(FAILED(hResult), L"Device [CreateDevice] | Failed to upgrade device context to ID3D11DeviceContext1.");
	}
#pragma endregion

#pragma region Factory
	Factory::Factory(Common::LoggerWide& logger) noexcept
		: m_Logger(logger)
	{
		m_Logger.LogInfoNL(L"Factory [()] | Constructed.");
	}

	Factory::~Factory() noexcept
	{
		if (m_Created)
			Release();

		m_Logger.LogInfoNL(L"Factory [~()] | Destroyed.");
	}

	void Factory::Create(Device& device) noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Created,
			L"Factory [Create] | Attempted to create factory after Factory has already been created."
		);

		m_Logger.LogFatalNLIf(
			!device.IsCreated(),
			L"Factory [Create] | Attempted to create factory before the provided Device was created."
		);

		HRESULT hResult = CreateDXGIFactory1(IID_PPV_ARGS(&mP_Factory));
		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"Factory [Create] | Failed to create factory : ",
			Win::Utility::TranslateError(hResult)
		);

		m_Logger.LogInfoNL(L"Factory [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void Factory::Release() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Created,
			L"Factory [Release] | Attempted to release factory before Factory has been created."
		);

		m_Logger.LogFatalNLIf(
			m_Released,
			L"Factory [Release] | Attempted to release factory after Factory has already been released."
		);

		mP_Factory.Reset();

		m_Logger.LogInfoNL(L"Factory [Release] | Released.");

		m_Created = false;
		m_Released = true;
	}

	IDXGIFactory1* Factory::operator->() noexcept
	{
		return GetRaw();
	}
#pragma endregion

#pragma region SwapChain
	SwapChain::SwapChain(Common::LoggerWide& logger) noexcept
		: m_Logger(logger)
	{
		m_Desc.BufferDesc.Width = 0;
		m_Desc.BufferDesc.Height = 0;
		m_Desc.BufferDesc.RefreshRate.Numerator = 0;
		m_Desc.BufferDesc.RefreshRate.Denominator = 0;

		//m_Desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_Desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Apparently use BGRA for Direct2D.

		m_Desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		m_Desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		m_Desc.SampleDesc.Count = 1;
		m_Desc.SampleDesc.Quality = 0;

		m_Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	
		m_Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		m_Desc.BufferCount = 2;

		m_Desc.Windowed = true;

		m_Desc.Flags = 0;

		m_Logger.LogInfoNL(L"SwapChain [()] | Constructed.");
	}

	SwapChain::~SwapChain() noexcept
	{
		if (m_Created)
			Release();

		m_Logger.LogInfoNL(L"SwapChain [~()] | Destroyed.");
	}

	void SwapChain::Create(const HWND hWnd, const RECT clientArea, Factory& factory, Device& device) noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Created,
			L"SwapChain [Create] | Attempted to create SwapChain after it has already been created."
		);

		m_Logger.LogFatalNLIf(
			!factory.IsCreated(),
			L"SwapChain [Create] | Attempted to create SwapChain before the provided Factory has been created."
		);

		m_Logger.LogFatalNLIf(
			!factory.IsCreated(),
			L"SwapChain [Create] | Attempted to create SwapChain before the provided Device has been created."
		);

		m_Desc.OutputWindow = hWnd;

		HRESULT hResult = factory->CreateSwapChain(device.GetRaw(), &m_Desc, &mP_SwapChain);

		m_Logger.LogFatalNLAppendIf(
			FAILED(hResult),
			L"SwapChain [Create] | Failed to create swap chain : ",
			Win::Utility::TranslateError(hResult)
		);
	
		m_Created = true;
		m_Released = false;

		m_Logger.LogInfoNL(L"SwapChain [Create] | Created.");
	}

	void SwapChain::Release() noexcept
	{
		m_Logger.LogFatalNLIf(
			!m_Created,
			L"SwapChain [Release] | Attempted to release SwapChain before it has been created."
		);

		m_Logger.LogFatalNLIf(
			m_Released,
			L"SwapChain [Release] | Attempted to release SwapChain after it has already been released."
		);

		DXGI_SWAP_CHAIN_DESC desc;
		HRESULT hResult = mP_SwapChain->GetDesc(&desc);

		m_Logger.LogFatalNLIf(
			FAILED(hResult),
			L"SwapChain [Release] | Failed to retrieve swap chain descriptor."
		);

		/* You must switch a swap chain out of exclusive fullscreen mode before releasing it ... */
		if (!desc.Windowed)
			mP_SwapChain->SetFullscreenState(false, nullptr);

		mP_SwapChain.Reset();

		m_Created = false;
		m_Released = true;

		m_Logger.LogInfoNL(L"SwapChain [Release] | Released.");
	}

	IDXGISwapChain* SwapChain::operator->() noexcept
	{
		return GetRaw();
	}
#pragma endregion

#pragma region InfoQueue
	InfoQueue::InfoQueue(Common::LoggerWide& logger) noexcept
		: m_Logger(logger)
	{
		m_Logger.LogInfoNL(L"SwapChain [()] | Constructed.");
	}

	InfoQueue::~InfoQueue() noexcept
	{
		if (m_Created)
			Release();

		m_Logger.LogInfoNL(L"InfoQueue [~()] | Destroyed.");
	}

	void InfoQueue::Create(Device& device) noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Created,
			L"InfoQueue [Create] | Attempted to create InfoQueue after creation has already occured."
		);

		m_Logger.LogFatalNLIf(
			!device.IsCreated(),
			L"InfoQueue [Create] | Attempted to create InfoQueue with a un-initialized device."
		);

		// Get the info queue interface.
		HRESULT hResult = device->QueryInterface(IID_PPV_ARGS(&mP_InfoQueue));
		m_Logger.LogFatalNLIf(
			FAILED(hResult),
			L"DXInfoQueue [Create] | Failed to retrieve an ID3D11InfoQueue, meaning no "
			L"debug output will be generated. Is the debug layer enabled ?"
		);

		m_Created = true;
		m_Released = false;

		m_Logger.LogInfoNL(L"InfoQueue [Create] | Created.");
	}

	void InfoQueue::Release() noexcept
	{
		m_Logger.LogFatalNLIf(
			m_Released,
			L"InfoQueue [Release] | Attempted to release after InfoQueue has already been released."
		);
		
		m_Logger.LogFatalNLIf(
			!m_Created,
			L"InfoQueue [Release] | Attempted to release before InfoQueue has been created."
		);

		mP_InfoQueue.Reset();

		m_Created = false;
		m_Released = true;

		m_Logger.LogInfoNL(L"InfoQueue [Release] | Released.");
	}

	void InfoQueue::LogMessages() noexcept
	{
		if (IsQueueEmpty())
			return;

		std::vector<std::wstring> messages;
		GetMessages(messages);

		for (const std::wstring& message : messages)
			m_Logger.LogInfoNLAppend(
				L"InfoQueue [LogMessages] | Debug message generated : ",
				message
			);
	}

	[[nodiscard]] bool InfoQueue::IsQueueEmpty() noexcept
	{
		bool notCreated = m_Logger.LogWarningNLIf(
			!m_Created,
			L"InfoQueue [IsQueueEmpty] | Attempted to check if info queue is empty "
			L"before InfoQueue is created."
		);

		if (notCreated)
			return true;

		return mP_InfoQueue->GetNumStoredMessages() == 0;
	}

	void InfoQueue::GetMessages(std::vector<std::wstring>& outMessages) noexcept
	{
		bool notCreated = m_Logger.LogWarningNLIf(
			!m_Created,
			L"InfoQueue [GetMessages] | Attempted to retrieve messages before InfoQueue "
			L"has been created."
		);

		if (notCreated)
			return;

		outMessages.reserve((size_t)mP_InfoQueue->GetNumStoredMessages());

		size_t messageLength = 0;

		HRESULT hResult = S_OK;
		for (size_t i = 0; i < mP_InfoQueue->GetNumStoredMessages(); ++i)
		{
			// Get size of message.
			hResult = mP_InfoQueue->GetMessage(i, nullptr, &messageLength);

			if (messageLength == 0)
				continue;

			std::unique_ptr<std::byte[]> pMessage(new std::byte[messageLength]);
			D3D11_MESSAGE* pRawMessage = reinterpret_cast<D3D11_MESSAGE*>(pMessage.get());

			hResult = mP_InfoQueue->GetMessage(i, pRawMessage, &messageLength);

			bool failed = m_Logger.LogWarningNLIf(
				FAILED(hResult),
				L"InfoQueue [GetMessages] | Failed to get a message in the info queue. "
				L"Skipping message..."
			);

			if (failed)
				continue;

			outMessages.emplace_back(pRawMessage->pDescription, pRawMessage->pDescription + pRawMessage->DescriptionByteLength);
		}
	}
#pragma endregion

#pragma region Writer
	//Writer::Writer(Common::LoggerWide& cmLogger) noexcept
	//	: m_Logger(cmLogger)
	//{
	//	m_Logger.LogInfoNL(L"Writer [()] | Constructed.");
	//}

	//Writer::~Writer() noexcept
	//{
	//	if (m_Created)
	//		Release();

	//	m_Logger.LogInfoNL(L"Writer [~()] | Destroyed.");
	//}

	//void Writer::Create(SwapChain& swapChain) noexcept
	//{
	//	m_Logger.LogFatalNLIf(m_Created, L"Writer [Create] | Attempted to recreate component.");
	//	m_Logger.LogFatalNLIf(!swapChain.IsCreated(), L"Writer [Create] | Swap chain was not created previously.");

	//	CreateIndependentResources();
	//	CreateDependentResources(swapChain);

	//	m_Created = true;
	//	m_Released = false;

	//	m_Logger.LogInfoNL(L"Writer [Create] | Created.");
	//}

	//void Writer::WriteText(std::wstring_view text, float layoutLeft, float layoutTop, float layoutRight, float layoutBottom) noexcept
	//{
	//	m_Logger.LogFatalNLIf(!m_Created, L"Writer [WriteText] | Attempted to write text before component was created.");

	//	D2D1_RECT_F layoutRect = D2D1::RectF(layoutLeft, layoutTop, layoutRight, layoutBottom);

	//	mP_D2DRenderTarget->BeginDraw();

	//	mP_D2DRenderTarget->DrawText(
	//		text.data(),
	//		static_cast<UINT32>(text.size()),
	//		mP_TextFormat.Get(),
	//		&layoutRect,
	//		mP_Brush.Get()
	//	);

	//	mP_D2DRenderTarget->EndDraw();
	//}

	//void Writer::CreateIndependentResources() noexcept
	//{
	//	HRESULT hResult = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, mP_D2DFactory.GetAddressOf());
	//	m_Logger.LogFatalNLAppendIf(
	//		hResult != S_OK,
	//		L"Writer [CreateIndependentResources] | Failed to create Direct2D factory : ",
	//		WindowsUtility::TranslateDWORDError(hResult)
	//	);

	//	hResult = DWriteCreateFactory(
	//		DWRITE_FACTORY_TYPE_SHARED,
	//		__uuidof(IDWriteFactory),
	//		reinterpret_cast<IUnknown**>(mP_DWriteFactory.GetAddressOf())
	//	);

	//	m_Logger.LogFatalNLAppendIf(
	//		hResult != S_OK,
	//		L"Writer [CreateIndependentResources] | Failed to create DWrite factory : ",
	//		WindowsUtility::TranslateDWORDError(hResult)
	//	);

	//	hResult = mP_DWriteFactory->CreateTextFormat(
	//		L"Gabriola",
	//		nullptr, // Font collection (NULL sets it to use the system font collection).
	//		DWRITE_FONT_WEIGHT_REGULAR,
	//		DWRITE_FONT_STYLE_NORMAL,
	//		DWRITE_FONT_STRETCH_NORMAL,
	//		72.0f,
	//		L"en-us", // Locale name.
	//		mP_TextFormat.GetAddressOf()
	//	);

	//	m_Logger.LogFatalNLAppendIf(
	//		hResult != S_OK,
	//		L"Writer [CreateIndependentResources] | Failed to create text format : ",
	//		WindowsUtility::TranslateDWORDError(hResult)
	//	);

	//	m_Logger.LogFatalNLAppendIf(
	//		(hResult = mP_TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER)) != S_OK,
	//		L"Writer [CreateIndependentResources] | Failed to create align text center : ",
	//		WindowsUtility::TranslateDWORDError(hResult)
	//	);

	//	m_Logger.LogFatalNLAppendIf(
	//		(hResult = mP_TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)) != S_OK,
	//		L"Writer [CreateIndependentResources] | Failed to create align paragraph center : ",
	//		WindowsUtility::TranslateDWORDError(hResult)
	//	);
	//}

	//void Writer::CreateDependentResources(SwapChain& swapChain) noexcept
	//{
	//	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;

	//	HRESULT hResult = S_OK;

	//	m_Logger.LogFatalNLAppendIf(
	//		(hResult = swapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()))) != S_OK,
	//		L"Writer [CreateDependentResources] | Failed to get back buffer of swap chain.", 
	//		WindowsUtility::TranslateDWORDError(hResult)
	//	);

	//	Microsoft::WRL::ComPtr<IDXGISurface> pSurface;
	//	m_Logger.LogFatalNLAppendIf(
	//		(hResult = pBackBuffer.As(&pSurface)) != S_OK,
	//		L"Writer [CreateDependentResources] | Failed to get GI surface from back buffer.",
	//		WindowsUtility::TranslateDWORDError(hResult)
	//	);

	//	// More here : https://learn.microsoft.com/en-us/windows/win32/direct2d/direct2d-and-direct3d-interoperation-overview
	//	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
	//		D2D1_RENDER_TARGET_TYPE_DEFAULT,
	//		D2D1::PixelFormat(
	//			DXGI_FORMAT_UNKNOWN,  // Typical swap chain format
	//			D2D1_ALPHA_MODE_UNKNOWN
	//		),
	//		0.0f,   // dpiX (use 0.0f to default to system DPI)
	//		0.0f,   // dpiY
	//		D2D1_RENDER_TARGET_USAGE_NONE,
	//		D2D1_FEATURE_LEVEL_DEFAULT
	//	);

	//	DXGI_SWAP_CHAIN_DESC desc;
	//	swapChain->GetDesc(&desc);
	//		
	//	hResult = mP_D2DFactory->CreateDxgiSurfaceRenderTarget(pSurface.Get(), props, mP_D2DRenderTarget.GetAddressOf());

	//	m_Logger.LogFatalNLAppendIf(
	//		hResult != S_OK,
	//		L"Writer [CreateDependentResources] | Failed to create render target from DXGI surface. ",
	//		WindowsUtility::TranslateDWORDError(hResult)
	//	);

	//	hResult = mP_D2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), mP_Brush.GetAddressOf());

	//	m_Logger.LogFatalNLAppendIf(
	//		hResult != S_OK,
	//		L"Writer [CreateDependentResources] | Failed to create brush.",
	//		WindowsUtility::TranslateDWORDError(hResult)
	//	);
	//}

	//void Writer::Release() noexcept
	//{
	//	if (m_Logger.LogWarningNLIf(m_Released, L"Writer [Create] | Attempted to re-release component."))
	//		return;
	//	else if (m_Logger.LogWarningNLIf(!m_Created, L"Writer [Create] | Attempted to release component before it was created."))
	//		return;
	//	
	//	// Technically, these are device independent - so they do not need to be released and recreated if the device is lost.
	//	// I don't care enough anyway...
	//	mP_D2DFactory.Reset();
	//	mP_DWriteFactory.Reset();
	//	mP_TextFormat.Reset();

	//	// These are device dependent, and do need to be released when the device becomes invalid.
	//	mP_D2DRenderTarget.Reset();
	//	mP_Brush.Reset();

	//	m_Created = false;
	//	m_Released = true;

	//	m_Logger.LogInfoNL(L"Writer [Release] | Released.");
	//}

#pragma endregion
}