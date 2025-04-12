#include "Core/CMPCH.hpp"
#include "Internal/Utility/WindowsUtility.hpp"
#include "Internal/DirectX/DXComponents.hpp"
#include "Internal/DirectX/DXUtility.hpp"

namespace CMRenderer::CMDirectX::Components
{
#pragma region DXDevice
	DXDevice::DXDevice(Utility::CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef)
	{
	}

	DXDevice::~DXDevice() noexcept
	{
		if (m_Created)
			Release();
	}

	void DXDevice::Create() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Created, L"DXDevice [Create] | Attempted to create after the device has already been created.");

		CreateDevice();

		m_CMLoggerRef.LogInfoNL(L"DXDevice [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void DXDevice::Release() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Created, L"DXDevice [Release] | Attempted to release before creation has occured.");
		m_CMLoggerRef.LogFatalNLIf(m_Released, L"DXDevice [Release] | Attempted to release after DXDevice was released previously.");

		// Apparently this is the best practice in order to avoid dangling references and what not,
		// I really don't know anymore...
		mP_Context->OMSetRenderTargets(0, nullptr, nullptr);
		mP_Context->ClearState();
		mP_Context->Flush();

		mP_Context.Reset();
		mP_Device.Reset();

		m_CMLoggerRef.LogInfoNL(L"DXDevice [Create] | Released.");

		m_Created = false;
		m_Released = true;
	}

	ID3D11Device* DXDevice::operator->() noexcept
	{
		return DeviceRaw();
	}

	void DXDevice::CreateDevice() noexcept
	{
		D3D11_CREATE_DEVICE_FLAG flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

		// flags | D3D11_CREATE_DEVICE_DEBUGGABLE // For complex debugging... more stuff here : https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_create_device_flag
		CM_IF_DEBUG(
			flags = static_cast<D3D11_CREATE_DEVICE_FLAG>(D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG)
		);

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
			&mP_Device,
			&succeededLevel,
			&mP_Context
		);

		m_CMLoggerRef.LogFatalNLAppendIf(
			hResult != S_OK,
			L"DXDevice [Create] | Failed to create device : ",
			WindowsUtility::TranslateDWORDError(hResult)
		);

		m_CMLoggerRef.LogFatalNLVariadicIf(
			succeededLevel < D3D_FEATURE_LEVEL_11_0,
			L"DXDevice [Create] | DirectX Feature Level was less than 11.0 (",
			CMDirectX::DXUtility::D3DFeatureLevelToWStrView(succeededLevel).data(),
			L"), which is less than required for this program. Continuation will only result in errors."
		);

		m_CMLoggerRef.LogInfoNLAppend(
			L"DXDevice [Create] | Feature level in use : ",
			CMDirectX::DXUtility::D3DFeatureLevelToWStrView(succeededLevel)
		);
	}
#pragma endregion

#pragma region DXFactory
	DXFactory::DXFactory(CMRenderer::Utility::CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef)
	{
	}

	DXFactory::~DXFactory() noexcept
	{
		if (m_Created)
			Release();
	}

	void DXFactory::Create(DXDevice& deviceRef) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Created, L"DXFactory [Create] | Attempted to create factory after DXFactory has already been created.");
		m_CMLoggerRef.LogFatalNLIf(!deviceRef.IsCreated(), L"DXFactory [Create] | Attempted to create factory before the provided DXDevice was created.");

		HRESULT hResult = CreateDXGIFactory1(IID_PPV_ARGS(&mP_Factory));

		m_CMLoggerRef.LogFatalNLAppendIf(hResult != S_OK, L"DXFactory [Create] | Failed to create factory : ", WindowsUtility::TranslateDWORDError(hResult));

		m_CMLoggerRef.LogInfoNL(L"DXFactory [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void DXFactory::Release() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(!m_Created, L"DXFactory [Release] | Attempted to release factory before DXFactory has been created.");
		m_CMLoggerRef.LogFatalNLIf(m_Released, L"DXFactory [Release] | Attempted to release factory after DXFactory has already been released.");

		mP_Factory.Reset();

		m_CMLoggerRef.LogInfoNL(L"DXFactory [Release] | Released.");

		m_Created = false;
		m_Released = true;
	}

	IDXGIFactory1* DXFactory::operator->() noexcept
	{
		return FactoryRaw();
	}
#pragma endregion

#pragma region DXSwapChain
	DXSwapChain::DXSwapChain(Utility::CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef)
	{
		m_Desc.BufferDesc.Width = 0;
		m_Desc.BufferDesc.Height = 0;
		m_Desc.BufferDesc.RefreshRate.Numerator = 0;
		m_Desc.BufferDesc.RefreshRate.Denominator = 0;
		m_Desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_Desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		m_Desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		m_Desc.SampleDesc.Count = 1;
		m_Desc.SampleDesc.Quality = 0;

		m_Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	
		m_Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		m_Desc.BufferCount = 2;
		m_Desc.Windowed = true;

		m_Desc.Flags = 0;
	}

	DXSwapChain::~DXSwapChain() noexcept
	{
		if (m_Created)
			Release();
	}

	void DXSwapChain::Create(const HWND hWnd, const RECT clientArea, DXFactory& factoryRef, DXDevice& deviceRef) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Created, L"DXSwapChain [Create] | Attempted to create DXSwapChain after it has already been created.");
		m_CMLoggerRef.LogFatalNLIf(!factoryRef.IsCreated(), L"DXSwapChain [Create] | Attempted to create DXSwapChain before the provided DXFactory has been created.");
		m_CMLoggerRef.LogFatalNLIf(!deviceRef.IsCreated(), L"DXSwapChain [Create] | Attempted to create DXSwapChain before the provided DXDevice has been created.");

		m_Desc.OutputWindow = hWnd;

		HRESULT hResult = factoryRef->CreateSwapChain(deviceRef.DeviceRaw(), &m_Desc, &mP_SwapChain);

		m_CMLoggerRef.LogFatalNLAppendIf(
			hResult != S_OK,
			L"DXSwapChain [Create] | Failed to create swap chain : ",
			WindowsUtility::TranslateDWORDError(hResult)
		);
	
		m_CMLoggerRef.LogInfoNL(L"DXSwapChain [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void DXSwapChain::Release() noexcept
	{
		m_CMLoggerRef.LogWarningNLIf(!m_Created, L"DXSwapChain [Release] | Attempted to release DXSwapChain before it has been created.");
		m_CMLoggerRef.LogWarningNLIf(m_Released, L"DXSwapChain [Release] | Attempted to release DXSwapChain after it has already been released.");

		mP_SwapChain.Reset();

		m_CMLoggerRef.LogInfoNL(L"DXSwapChain [Release] | Released.");

		m_Created = false;
		m_Released = true;
	}

	IDXGISwapChain* DXSwapChain::operator->() noexcept
	{
		return SwapChainRaw();
	}
#pragma endregion

#pragma region DXInfoQueue
	DXInfoQueue::DXInfoQueue(Utility::CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef)
	{
	}

	DXInfoQueue::~DXInfoQueue() noexcept
	{
		if (m_Created)
			Release();
	}

	void DXInfoQueue::Create(DXDevice& deviceRef) noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Created, L"DXInfoQueue [Create] | Attempted to create DXInfoQueue after creation has already occured.");
		m_CMLoggerRef.LogFatalNLIf(!deviceRef.IsCreated(), L"DXInfoQueue [Create] | Attempted to create DXInfoQueue with a un-initialized device.");

		// Get the info queue interface.
		HRESULT hResult = deviceRef->QueryInterface(IID_PPV_ARGS(&mP_InfoQueue));
		m_CMLoggerRef.LogFatalNLIf(hResult != S_OK, L"DXInfoQueue [Create] | Failed to retrieve an ID3D11InfoQueue, meaning no debug output will be generated. Is the debug layer enabled?");

		m_CMLoggerRef.LogInfoNL(L"DXInfoQueue [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void DXInfoQueue::Release() noexcept
	{
		m_CMLoggerRef.LogFatalNLIf(m_Released, L"DXInfoQueue [Release] | Attempted to release after DXInfoQueue has already been released.");
		m_CMLoggerRef.LogFatalNLIf(!m_Created, L"DXInfoQueue [Release] | Attempted to release before DXInfoQueue has been created.");

		mP_InfoQueue.Reset();

		m_CMLoggerRef.LogInfoNL(L"DXInfoQueue [Release] | Released.");

		m_Created = false;
		m_Released = true;
	}

	void DXInfoQueue::LogMessages() noexcept
	{
		if (IsQueueEmpty())
			return;

		std::vector<std::wstring> messages;
		GetMessages(messages);

		for (const std::wstring& message : messages)
			m_CMLoggerRef.LogInfoNLAppend(
				L"DXInfoQueue [LogMessages] | Debug message generated : ",
				message
			);
	}

	[[nodiscard]] bool DXInfoQueue::IsQueueEmpty() noexcept
	{
		if (m_CMLoggerRef.LogWarningNLIf(!m_Created, L"DXInfoQueue [IsQueueEmpty] | Attempted to check if info queue is empty before DXInfoQueue is created."))
			return true;

		return mP_InfoQueue->GetNumStoredMessages() == 0;
	}

	void DXInfoQueue::GetMessages(std::vector<std::wstring>& outMessages) noexcept
	{
		if (m_CMLoggerRef.LogWarningNLIf(!m_Created, L"DXInfoQueue [GetMessages] | Attempted to retrieve messages before DXInfoQueue has been created."))
			return;

		outMessages.reserve((size_t)mP_InfoQueue->GetNumStoredMessages());

		size_t messageLength = 0;
		HRESULT hResult = S_OK;
		for (size_t i = 0; i < mP_InfoQueue->GetNumStoredMessages(); ++i)
		{
			// Get size of message.
			HRESULT hResult = mP_InfoQueue->GetMessageW(i, nullptr, &messageLength);

			if (messageLength == 0)
				continue;

			std::unique_ptr<std::byte[]> pMessage(new std::byte[messageLength]);
			D3D11_MESSAGE* pRawMessage = reinterpret_cast<D3D11_MESSAGE*>(pMessage.get());

			hResult = mP_InfoQueue->GetMessageW(i, pRawMessage, &messageLength);

			if (m_CMLoggerRef.LogWarningNLIf(hResult != S_OK, L"DXInfoQueue [GetMessages] | Failed to get a message in the info queue. Skipping message..."))
				continue;

			outMessages.emplace_back(pRawMessage->pDescription, pRawMessage->pDescription + pRawMessage->DescriptionByteLength);
		}
	}
#pragma endregion

#pragma region DXWriter
	DXWriter::DXWriter(Utility::CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef)
	{
	}

	DXWriter::~DXWriter() noexcept
	{
		if (m_Created)
			Release();

		m_CMLoggerRef.LogInfoNL(L"DXWriter [~()] | Destroyed.");
	}

	void DXWriter::Create(DXSwapChain& swapChainRef) noexcept
	{
		if (m_CMLoggerRef.LogWarningNLIf(m_Created, L"DXWriter [Create] | Attempted to recreate component."))
			return;

		m_CMLoggerRef.LogFatalNLIf(!swapChainRef.IsCreated(), L"DXWriter [Create] | Swap chain was not created previously.");

		CreateIndependentResources();
		CreateDependentResources(swapChainRef);

		m_Created = true;
		m_Released = false;
	}

	void DXWriter::WriteText(std::wstring_view text) noexcept
	{
		if (m_CMLoggerRef.LogWarningNLIf(!m_Created, L"DXWriter [WriteText] | Attempted to write text before component was created."))
			return;

	}

	void DXWriter::CreateIndependentResources() noexcept
	{
		HRESULT hResult = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, mP_D2DFactory.GetAddressOf());
		m_CMLoggerRef.LogFatalNLAppendIf(
			hResult != S_OK,
			L"DXWriter [CreateIndependentResources] | Failed to create Direct2D factory : ",
			WindowsUtility::TranslateDWORDError(hResult)
		);

		hResult = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(mP_DWriteFactory.GetAddressOf())
		);

		m_CMLoggerRef.LogFatalNLAppendIf(
			hResult != S_OK,
			L"DXWriter [CreateIndependentResources] | Failed to create DWrite factory : ",
			WindowsUtility::TranslateDWORDError(hResult)
		);

		hResult = mP_DWriteFactory->CreateTextFormat(
			L"Gabriola",
			nullptr, // Font collection (NULL sets it to use the system font collection).
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			72.0f,
			L"en-us", // Locale name.
			mP_TextFormat.GetAddressOf()
		);

		m_CMLoggerRef.LogFatalNLAppendIf(
			hResult != S_OK,
			L"DXWriter [CreateIndependentResources] | Failed to create text format : ",
			WindowsUtility::TranslateDWORDError(hResult)
		);

		m_CMLoggerRef.LogFatalNLAppendIf(
			(hResult = mP_TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER)) != S_OK,
			L"DXWriter [CreateIndependentResources] | Failed to create align text center : ",
			WindowsUtility::TranslateDWORDError(hResult)
		);

		m_CMLoggerRef.LogFatalNLAppendIf(
			(hResult = mP_TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)) != S_OK,
			L"DXWriter [CreateIndependentResources] | Failed to create align paragraph center : ",
			WindowsUtility::TranslateDWORDError(hResult)
		);
	}

	void DXWriter::CreateDependentResources(DXSwapChain& swapChainRef) noexcept
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;

		HRESULT hResult = S_OK;

		m_CMLoggerRef.LogFatalNLAppendIf(
			(hResult = swapChainRef->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()))) != S_OK,
			L"DXWriter [CreateDependentResources] | Failed to get back buffer of swap chain.", 
			WindowsUtility::TranslateDWORDError(hResult)
		);

		Microsoft::WRL::ComPtr<IDXGISurface> pSurface;
		m_CMLoggerRef.LogFatalNLAppendIf(
			(hResult = pBackBuffer.As(&pSurface)) != S_OK,
			L"DXWriter [CreateDependentResources] | Failed to get DXGI surface from back buffer.",
			WindowsUtility::TranslateDWORDError(hResult)
		);

		// More here : https://learn.microsoft.com/en-us/windows/win32/direct2d/direct2d-and-direct3d-interoperation-overview
		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_STRAIGHT),
			96,
			96
		);

		hResult = mP_D2DFactory->CreateDxgiSurfaceRenderTarget(pSurface.Get(), props, mP_D2DRenderTarget.GetAddressOf());

		m_CMLoggerRef.LogFatalNLAppendIf(
			hResult != S_OK,
			L"DXWriter [CreateDependentResources] | Failed to create render target from DXGI surface.",
			WindowsUtility::TranslateDWORDError(hResult)
		);

		hResult = mP_D2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), mP_Brush.GetAddressOf());

		m_CMLoggerRef.LogFatalNLAppendIf(
			hResult != S_OK,
			L"DXWriter [CreateDependentResources] | Failed to create brush.",
			WindowsUtility::TranslateDWORDError(hResult)
		);
	}

	void DXWriter::Release() noexcept
	{
		if (m_CMLoggerRef.LogWarningNLIf(m_Released, L"DXWriter [Create] | Attempted to re-release component."))
			return;
		else if (m_CMLoggerRef.LogWarningNLIf(!m_Created, L"DXWriter [Create] | Attempted to release component before it was created."))
			return;
		
		// Technically, these are device independent - so they do not need to be released and recreated if the device is lost.
		// I don't care enough anyway...
		mP_D2DFactory.Reset();
		mP_DWriteFactory.Reset();
		mP_TextFormat.Reset();

		// These are device dependent, and do need to be released when the device becomes invalid.
		mP_D2DRenderTarget.Reset();
		mP_Brush.Reset();

		m_Created = false;
		m_Released = true;
	}

#pragma endregion
}