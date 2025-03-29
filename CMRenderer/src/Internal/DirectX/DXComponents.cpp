#include "Core/CMPCH.hpp"
#include "Internal/DirectX/DXComponents.hpp"
#include "Internal/DirectX/DXUtility.hpp"
#include "Internal/WindowsUtility.hpp"

namespace CMRenderer::DirectX::Components
{
#pragma region DXDevice
	DXDevice::DXDevice(CMLoggerWide& cmLoggerRef) noexcept
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
		if (m_Created)
		{
			m_CMLoggerRef.LogWarning(L"DXDevice [Create] | Attempted to create after DXDevice has already been created.\n");
			return;
		}

		CreateDevice();

		m_CMLoggerRef.LogInfo(L"DXDevice [Create] | Created.\n");

		m_Created = true;
		m_Released = false;
	}

	void DXDevice::Release() noexcept
	{
		if (!m_Created)
		{
			m_CMLoggerRef.LogWarning(L"DXDevice [Release] | Attempted to release before creation has occured.\n");
			return;
		}
		else if (m_Released)
		{
			m_CMLoggerRef.LogWarning(L"DXDevice [Release] | Attempted to release after DXDevice was released previously.\n");
			return;
		}

		mP_Context.Reset();
		mP_Device.Reset();

		m_CMLoggerRef.LogInfo(L"DXDevice [Create] | Released.\n");

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

		if (hResult != S_OK)
		{
			std::wstring message = L"DXDevice [Create] | Failed to create device : " +
				WindowsUtility::TranslateDWORDError(hResult) + L'\n';

			m_CMLoggerRef.LogFatal(message);
			return;
		}
		else if (succeededLevel < D3D_FEATURE_LEVEL_11_0)
		{
			std::wstring message = L"DXDevice [Create] | DirectX Feature Level was less than 11.0 (" +
				std::wstring(DirectX::Utility::D3DFeatureLevelToWStrView(succeededLevel).data()) +
				L"), which is less than required for this program. Continuation will only result in errors.\n";

			m_CMLoggerRef.LogFatal(message);
			return;
		}

		std::wstring message = L"DXDevice [Create] | Feature level in use : " +
			std::wstring(DirectX::Utility::D3DFeatureLevelToWStrView(succeededLevel)) + L'\n';

		m_CMLoggerRef.LogInfo(message);
	}
#pragma endregion

#pragma region DXFactory
DXFactory::DXFactory(CMLoggerWide& cmLoggerRef) noexcept
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
	if (m_Created)
	{
		m_CMLoggerRef.LogWarning(L"DXFactory [Create] | Attempted to create factory after DXFactory has already been created.\n");
		return;
	}

	if (!deviceRef.IsCreated())
	{
		m_CMLoggerRef.LogFatal(L"DXFactory [Create] | Attempted to create factory before the provided DXDevice was created.\n");
		return;
	}

	Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice;
	HRESULT hResult = deviceRef->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));

	if (hResult != S_OK)
	{
		std::wstring message = L"DXFactory [Create] | Failed to retrieve IDXGIDevice interface from ID3D11Device : " +
			WindowsUtility::TranslateDWORDError(hResult) + L'\n';

		m_CMLoggerRef.LogFatal(message);
		return;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
	hResult = pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter));

	if (hResult != S_OK)
	{
		std::wstring message = L"DXFactory [Create] | Failed to retrieve adapter from the IDXGIDevice interface : " +
			WindowsUtility::TranslateDWORDError(hResult) + L'\n';

		m_CMLoggerRef.LogFatal(message);
		return;
	}

	hResult = pDXGIAdapter->GetParent(IID_PPV_ARGS(&mP_Factory));

	if (hResult != S_OK)
	{
		std::wstring message = L"DXFactory [Create] | Failed to create DXGIFactory1 : " +
			WindowsUtility::TranslateDWORDError(hResult) + L'\n';

		m_CMLoggerRef.LogFatal(message);
		return;
	}

	m_CMLoggerRef.LogInfo(L"DXFactory [Create] | Created.\n");

	m_Created = true;
	m_Released = false;
}

void DXFactory::Release() noexcept
{
	if (!m_Created)
	{
		m_CMLoggerRef.LogWarning(L"DXFactory [Release] | Attempted to release factory before DXFactory has been created.\n");
		return;
	}
	else if (m_Released)
	{
		m_CMLoggerRef.LogWarning(L"DXFactory [Release] | Attempted to release factory after DXFactory has already been released.\n");
		return;
	}

	mP_Factory.Reset();

	m_CMLoggerRef.LogInfo(L"DXFactory [Release] | Released.\n");

	m_Created = false;
	m_Released = true;
}

IDXGIFactory1* DXFactory::operator->() noexcept
{
	return FactoryRaw();
}
#pragma endregion

#pragma region DXSwapChain
DXSwapChain::DXSwapChain(CMLoggerWide& cmLoggerRef) noexcept
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
}

DXSwapChain::~DXSwapChain() noexcept
{
	if (m_Created)
		Release();
}

void DXSwapChain::Create(const HWND hWnd, const RECT clientArea, DXFactory& factoryRef, DXDevice& deviceRef, bool isFullscreen) noexcept
{
	if (m_Created)
	{
		m_CMLoggerRef.LogWarning(L"DXSwapChain [Create] | Attempted to create DXSwapChain after it has already been created.\n");
		return;
	}

	if (!factoryRef.IsCreated())
	{
		m_CMLoggerRef.LogFatal(L"DXSwapChain [Create] | Attempted to create DXSwapChain before the provided DXFactory has been created.\n");
		return;
	}
	else if (!deviceRef.IsCreated())
	{
		m_CMLoggerRef.LogFatal(L"DXSwapChain [Create] | Attempted to create DXSwapChain before the provided DXDevice has been created.\n");
		return;
	}

	m_Desc.OutputWindow = hWnd;

	HRESULT hResult = factoryRef->CreateSwapChain(deviceRef.DeviceRaw(), &m_Desc, &mP_SwapChain);

	if (hResult != S_OK)
	{
		std::wstring message = L"DXContext [Init] | Failed to create swap chain : " +
			WindowsUtility::TranslateDWORDError(hResult) + L'\n';

		m_CMLoggerRef.LogFatal(message);
		return;
	}
	
	if (isFullscreen)
	{
		hResult = mP_SwapChain->SetFullscreenState(isFullscreen, nullptr);

		if (hResult != S_OK)
		{
			std::wstring message = L"DXContext [Init] | Failed to set swap chain to fullscreen : " +
				WindowsUtility::TranslateDWORDError(hResult) + L'\n';

			m_CMLoggerRef.LogFatal(message);
			return;
		}
	}

	m_CMLoggerRef.LogInfo(L"DXSwapChain [Create] | Created.\n");

	m_Created = true;
	m_Released = false;
}

void DXSwapChain::Release() noexcept
{
	if (!m_Created)
	{
		m_CMLoggerRef.LogWarning(L"DXSwapChain [Release] | Attempted to release DXSwapChain before it has been created.\n");
		return;
	}
	else if (m_Released)
	{
		m_CMLoggerRef.LogWarning(L"DXSwapChain [Release] | Attempted to release DXSwapChain after it has already been released.\n");
		return;
	}

	mP_SwapChain.Reset();

	m_CMLoggerRef.LogInfo(L"DXSwapChain [Release] | Released.\n");

	m_Created = false;
	m_Released = true;
}

IDXGISwapChain* DXSwapChain::operator->() noexcept
{
	return SwapChainRaw();
}
#pragma endregion

#pragma region DXInfoQueue
	DXInfoQueue::DXInfoQueue(CMLoggerWide& cmLoggerRef) noexcept
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
		if (m_Created)
		{
			m_CMLoggerRef.LogWarning(L"DXInfoQueue [Create] | Attempted to create DXInfoQueue after creation has already occured.\n");
			return;
		}

		if (!deviceRef.IsCreated())
		{
			m_CMLoggerRef.LogFatal(L"DXInfoQueue [Create] | Attempted to create DXInfoQueue with a un-initialized device.\n");
			return;
		}

		// Get the info queue interface.
		HRESULT hResult = deviceRef->QueryInterface(IID_PPV_ARGS(&mP_InfoQueue));

		if (hResult != S_OK)
		{
			m_CMLoggerRef.LogWarning(L"DXInfoQueue [Create] | Failed to retrieve an ID3D11InfoQueue, meaning no debug output will be generated. Is the debug layer enabled?\n");
			return;
		}

		m_CMLoggerRef.LogInfo(L"DXInfoQueue [Create] | Created.\n");

		m_Created = true;
		m_Released = false;
	}

	void DXInfoQueue::Release() noexcept
	{
		if (m_Released)
		{
			m_CMLoggerRef.LogWarning(L"DXInfoQueue [Release] | Attempted to release after DXInfoQueue has already been released.\n");
			return;
		}
		else if (!m_Created)
		{
			m_CMLoggerRef.LogWarning(L"DXInfoQueue [Release] | Attempted to release before DXInfoQueue has been created.\n");
			return;
		}

		mP_InfoQueue.Reset();

		m_CMLoggerRef.LogInfo(L"DXInfoQueue [Release] | Released.\n");

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
		{
			m_CMLoggerRef.LogInfo(L"DXInfoQueue [LogMessages] | Debug message generated : ");
			m_CMLoggerRef.LogInline(message);
			m_CMLoggerRef.LogInline(L"\n");
		}
	}

	[[nodiscard]] bool DXInfoQueue::IsQueueEmpty() noexcept
	{
		if (!m_Created)
		{
			m_CMLoggerRef.LogWarning(L"DXInfoQueue [IsQueueEmpty] | Attempted to check if info queue is empty before DXInfoQueue is created.\n");
			return true;
		}

		return mP_InfoQueue->GetNumStoredMessages() == 0;
	}

	void DXInfoQueue::GetMessages(std::vector<std::wstring>& outMessages) noexcept
	{
		if (!m_Created)
		{
			m_CMLoggerRef.LogWarning(L"DXInfoQueue [GetMessages] | Attempted to retrieve messages before DXInfoQueue has been created.\n");
			return;
		}

		outMessages.reserve((size_t)mP_InfoQueue->GetNumStoredMessages());

		size_t messageLength = 0;
		HRESULT hResult = S_OK;
		for (size_t i = 0; i < mP_InfoQueue->GetNumStoredMessages(); i++)
		{
			// Get size of message.
			HRESULT hResult = mP_InfoQueue->GetMessageW(i, nullptr, &messageLength);

			if (hResult != S_OK)
			{
				m_CMLoggerRef.LogWarning(L"DXInfoQueue [GetMessages] | Failed to retrieve message.\n");
				continue;
			}

			if (messageLength == 0)
				continue;

			std::unique_ptr<std::byte[]> pMessage(new std::byte[messageLength]);
			D3D11_MESSAGE* pRawMessage = reinterpret_cast<D3D11_MESSAGE*>(pMessage.get());

			hResult = mP_InfoQueue->GetMessageW(i, pRawMessage, &messageLength);

			if (hResult != S_OK)
			{
				m_CMLoggerRef.LogWarning(L"DXInfoQueue [GetMessages] | Failed to get a message in the info queue. Skipping message...\n");
				continue;
			}

			outMessages.emplace_back(pRawMessage->pDescription, pRawMessage->pDescription + pRawMessage->DescriptionByteLength);
		}
	}
#pragma endregion
}