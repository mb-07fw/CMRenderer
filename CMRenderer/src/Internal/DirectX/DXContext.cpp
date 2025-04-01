#include "Core/CMPCH.hpp"
#include "Internal/Utility/WindowsUtility.hpp"
#include "Internal/DirectX/DXContext.hpp"
#include "Internal/DirectX/DXUtility.hpp"

#include <comdef.h>

#define APPEND_COMMA(x) x,

namespace CMRenderer::CMDirectX
{
#pragma region DXContext
	DXContext::DXContext(Utility::CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef), m_Device(cmLoggerRef),
		  m_Factory(cmLoggerRef), m_SwapChain(cmLoggerRef),
		  CM_IF_NDEBUG_REPLACE(APPEND_COMMA(m_InfoQueue(cmLoggerRef)))
		  m_ShaderLibrary(cmLoggerRef)
	{
		CM_IF_DEBUG(
			m_DebugInterfaceModule = LoadLibrary(L"Dxgidebug.dll");
		
			if (m_DebugInterfaceModule == nullptr)
			{
				m_CMLoggerRef.LogFatalNL(L"DXContext [Shutdown] | Failed to load Dxgidebug.dll");
				return;
			}

			typedef HRESULT(WINAPI* DXGIGetDebugInterfaceFunc)(const IID&, void**);

			// Retrieve the address of DXGIGetDebugInterface function
			DXGIGetDebugInterfaceFunc pDXGIGetDebugInterface =
				(DXGIGetDebugInterfaceFunc)GetProcAddress(m_DebugInterfaceModule, "DXGIGetDebugInterface");

			if (!pDXGIGetDebugInterface)
			{
				m_CMLoggerRef.LogFatalNL(L"DXContext [Shutdown] | Failed to get function address for the DXGIGetDebugInterface.");
				return;
			}

			HRESULT hResult = pDXGIGetDebugInterface(IID_PPV_ARGS(&mP_DebugInterface));

			if (hResult != S_OK)
				m_CMLoggerRef.LogFatalNL(L"DXContext [Shutdown] | Failed to retrieve a DXGI debug interface.");
		);
	}

	DXContext::~DXContext() noexcept
	{
		if (m_Initialized)
			Shutdown();

		CM_IF_DEBUG(FreeLibrary(m_DebugInterfaceModule));
	}

	void DXContext::Init(const HWND hWnd, RECT clientArea) noexcept
	{
		if (m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"DXContext [Init] | Initializion has been attempted after CMRenderContext has already been initialized.");
			return;
		}

		m_Device.Create();
		m_Factory.Create(m_Device);
		m_SwapChain.Create(hWnd, clientArea, m_Factory, m_Device);

		CM_IF_DEBUG(
			m_InfoQueue.Create(m_Device);
			if (!m_InfoQueue.IsCreated())
			{
				m_CMLoggerRef.LogFatalNL(L"DXContext [Init] | Failed to initialize info queue.");
				return;
			}
		);

		m_ShaderLibrary.Init();

		CreateRTV();
		SetViewport((float)clientArea.right, (float)clientArea.bottom);
		SetTopology();

		m_CMLoggerRef.LogInfoNL(L"DXContext [Init] | Initialized.");

		m_Initialized = true;
		m_Shutdown = false;
	}

	void DXContext::Shutdown() noexcept
	{
		if (m_Shutdown)
		{
			m_CMLoggerRef.LogWarningNL(L"DXContext [Shutdown] | Shutdown has been attempted after shutdown has already occured previously.");
			return;
		}

		else if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"DXContext [Shutdown] | Shutdown has been attempted before initialization.");
			return;
		}

		mP_RTV.Reset();
		m_Factory.Release();
		m_Device.Release();
		m_SwapChain.Release();

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
				m_InfoQueue.LogMessages();

			m_InfoQueue.Release()
		);

		m_ShaderLibrary.Shutdown();

		m_Initialized = false;
		m_Shutdown = true;
	}

	void DXContext::Clear(NormColor normColor) noexcept
	{
		m_Device.Context()->ClearRenderTargetView(mP_RTV.Get(), normColor.rgba);
	}

	void DXContext::Present() noexcept
	{
		HRESULT hResult = m_SwapChain->Present(1, 0);

		if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
			);

			m_CMLoggerRef.LogFatalNLAppend(
				L"DXContext [Present] | Device removed error : ",
				WindowsUtility::TranslateDWORDError(hResult)
			);
		}
		else if (hResult != S_OK)
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
			);

			m_CMLoggerRef.LogFatalNLAppend(
				L"DXContext [Present] | Present error : ", 
				WindowsUtility::TranslateDWORDError(hResult)
			);
		}
	}

	void DXContext::CreateRTV() noexcept
	{
		ReleaseViews();

		// Get the back buffer.
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		HRESULT hResult = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(
				L"DXContext [CreateRTV] | Failed to get back buffer : ",
				WindowsUtility::TranslateDWORDError(hResult)
			);

		// Create the RTV.
		hResult = m_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, mP_RTV.GetAddressOf());

		if (hResult != S_OK)
			m_CMLoggerRef.LogFatalNLAppend(
				L"DXContext [CreateRTV] | Failed to create the Render Target View : ",
				WindowsUtility::TranslateDWORDError(hResult)
			);
	}

	void DXContext::SetViewport(float width, float height) noexcept
	{
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, width, height);
		m_Device.ContextRaw()->RSSetViewports(1, &viewport);
	}

	void DXContext::SetTopology() noexcept
	{
		m_Device.ContextRaw()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void DXContext::ReleaseViews() noexcept
	{
		m_Device.ContextRaw()->OMSetRenderTargets(0, nullptr, nullptr);
		mP_RTV.Reset();
	}
#pragma endregion
}