#include "Core/CMPCH.hpp"
#include "Internal/DirectX/DXComponents.hpp"
#include "Internal/DirectX/DXUtility.hpp"
#include "Internal/WindowsUtility.hpp"

namespace CMRenderer::CMDirectX::Components
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
			m_CMLoggerRef.LogWarningNL(L"DXDevice [Create] | Attempted to create after DXDevice has already been created.");
			return;
		}

		CreateDevice();

		m_CMLoggerRef.LogInfoNL(L"DXDevice [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void DXDevice::Release() noexcept
	{
		if (!m_Created)
		{
			m_CMLoggerRef.LogWarningNL(L"DXDevice [Release] | Attempted to release before creation has occured.");
			return;
		}
		else if (m_Released)
		{
			m_CMLoggerRef.LogWarningNL(L"DXDevice [Release] | Attempted to release after DXDevice was released previously.");
			return;
		}

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

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(
				L"DXDevice [Create] | Failed to create device : ",
				WindowsUtility::TranslateDWORDError(hResult)
		);
		else if (succeededLevel < D3D_FEATURE_LEVEL_11_0)
			m_CMLoggerRef.LogFatalNLVariadic(
				L"DXDevice [Create] | DirectX Feature Level was less than 11.0 (",
				CMDirectX::Utility::D3DFeatureLevelToWStrView(succeededLevel).data(),
				L"), which is less than required for this program. Continuation will only result in errors."
			);

		m_CMLoggerRef.LogInfoNLAppend(
			L"DXDevice [Create] | Feature level in use : ",
			CMDirectX::Utility::D3DFeatureLevelToWStrView(succeededLevel)
		);
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
			m_CMLoggerRef.LogWarningNL(L"DXFactory [Create] | Attempted to create factory after DXFactory has already been created.");
			return;
		}

		if (!deviceRef.IsCreated())
			m_CMLoggerRef.LogFatalNL(L"DXFactory [Create] | Attempted to create factory before the provided DXDevice was created.");

		HRESULT hResult = CreateDXGIFactory1(IID_PPV_ARGS(&mP_Factory));

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNL(L"DXFactory [Create] | Failed to create factory.");

		m_CMLoggerRef.LogInfoNL(L"DXFactory [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void DXFactory::Release() noexcept
	{
		if (!m_Created)
		{
			m_CMLoggerRef.LogWarningNL(L"DXFactory [Release] | Attempted to release factory before DXFactory has been created.");
			return;
		}
		else if (m_Released)
		{
			m_CMLoggerRef.LogWarningNL(L"DXFactory [Release] | Attempted to release factory after DXFactory has already been released.");
			return;
		}

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

		m_Desc.Flags = 0;
	}

	DXSwapChain::~DXSwapChain() noexcept
	{
		if (m_Created)
			Release();
	}

	void DXSwapChain::Create(const HWND hWnd, const RECT clientArea, DXFactory& factoryRef, DXDevice& deviceRef) noexcept
	{
		if (m_Created)
		{
			m_CMLoggerRef.LogWarningNL(L"DXSwapChain [Create] | Attempted to create DXSwapChain after it has already been created.");
			return;
		}

		if (!factoryRef.IsCreated())
			m_CMLoggerRef.LogFatalNL(L"DXSwapChain [Create] | Attempted to create DXSwapChain before the provided DXFactory has been created.");
		else if (!deviceRef.IsCreated())
			m_CMLoggerRef.LogFatalNL(L"DXSwapChain [Create] | Attempted to create DXSwapChain before the provided DXDevice has been created.");

		m_Desc.OutputWindow = hWnd;

		HRESULT hResult = factoryRef->CreateSwapChain(deviceRef.DeviceRaw(), &m_Desc, &mP_SwapChain);

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(
				L"DXSwapChain [Create] | Failed to create swap chain : ",
				WindowsUtility::TranslateDWORDError(hResult)
			);
	
		m_CMLoggerRef.LogInfoNL(L"DXSwapChain [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void DXSwapChain::Release() noexcept
	{
		if (!m_Created)
		{
			m_CMLoggerRef.LogWarningNL(L"DXSwapChain [Release] | Attempted to release DXSwapChain before it has been created.");
			return;
		}
		else if (m_Released)
		{
			m_CMLoggerRef.LogWarningNL(L"DXSwapChain [Release] | Attempted to release DXSwapChain after it has already been released.");
			return;
		}

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
			m_CMLoggerRef.LogWarningNL(L"DXInfoQueue [Create] | Attempted to create DXInfoQueue after creation has already occured.");
			return;
		}

		if (!deviceRef.IsCreated())
			m_CMLoggerRef.LogFatalNL(L"DXInfoQueue [Create] | Attempted to create DXInfoQueue with a un-initialized device.");

		// Get the info queue interface.
		HRESULT hResult = deviceRef->QueryInterface(IID_PPV_ARGS(&mP_InfoQueue));

		if (hResult != S_OK)
		{
			m_CMLoggerRef.LogWarningNL(L"DXInfoQueue [Create] | Failed to retrieve an ID3D11InfoQueue, meaning no debug output will be generated. Is the debug layer enabled?");
			return;
		}

		m_CMLoggerRef.LogInfoNL(L"DXInfoQueue [Create] | Created.");

		m_Created = true;
		m_Released = false;
	}

	void DXInfoQueue::Release() noexcept
	{
		if (m_Released)
		{
			m_CMLoggerRef.LogWarningNL(L"DXInfoQueue [Release] | Attempted to release after DXInfoQueue has already been released.");
			return;
		}
		else if (!m_Created)
		{
			m_CMLoggerRef.LogWarningNL(L"DXInfoQueue [Release] | Attempted to release before DXInfoQueue has been created.");
			return;
		}

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
		if (!m_Created)
		{
			m_CMLoggerRef.LogWarningNL(L"DXInfoQueue [IsQueueEmpty] | Attempted to check if info queue is empty before DXInfoQueue is created.");
			return true;
		}

		return mP_InfoQueue->GetNumStoredMessages() == 0;
	}

	void DXInfoQueue::GetMessages(std::vector<std::wstring>& outMessages) noexcept
	{
		if (!m_Created)
		{
			m_CMLoggerRef.LogWarningNL(L"DXInfoQueue [GetMessages] | Attempted to retrieve messages before DXInfoQueue has been created.");
			return;
		}

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

			if (hResult != S_OK)
			{
				m_CMLoggerRef.LogWarningNL(L"DXInfoQueue [GetMessages] | Failed to get a message in the info queue. Skipping message...");
				continue;
			}

			outMessages.emplace_back(pRawMessage->pDescription, pRawMessage->pDescription + pRawMessage->DescriptionByteLength);
		}
	}
#pragma endregion
}