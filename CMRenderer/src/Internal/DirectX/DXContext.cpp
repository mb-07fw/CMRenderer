#include "Core/CMPCH.hpp"
#include "Internal/WindowsUtility.hpp"
#include "Internal/DirectX/DXContext.hpp"
#include "Internal/DirectX/DXUtility.hpp"

namespace CMRenderer::DirectX
{
#pragma region DXContext
	DXContext::DXContext(CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef), m_Device(cmLoggerRef),
		  m_Factory(cmLoggerRef), m_SwapChain(cmLoggerRef),
		  m_InfoQueue(cmLoggerRef), m_ShaderLibrary(cmLoggerRef)
	{
	}

	DXContext::~DXContext() noexcept
	{
		if (m_Initialized)
			Shutdown();
	}

	void DXContext::Init(const HWND hWnd, const RECT clientArea, const bool isFullscreen) noexcept
	{
		if (m_Initialized)
		{
			m_CMLoggerRef.LogWarning(L"DXContext [Init] | Initializion has been attempted after CMRenderContext has already been initialized.\n");
			return;
		}

		m_Device.Create();
		m_Factory.Create(m_Device);
		m_SwapChain.Create(hWnd, clientArea, m_Factory, m_Device, isFullscreen);
		m_InfoQueue.Create(m_Device);
		
		HRESULT hResult = S_OK;

		// Get the back buffer.
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		hResult = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		if (hResult != S_OK)
		{
			std::wstring message = L"DXContext [Init] | Failed to get back buffer : " +
				WindowsUtility::TranslateDWORDError(hResult) + L'\n';

			m_CMLoggerRef.LogFatal(message);
		}

		// Create the RTV.
		hResult = m_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, mP_RTV.GetAddressOf());

		if (hResult != S_OK)
		{
			std::wstring message = L"DXContext [Init] | Failed to create the render target view : " +
				WindowsUtility::TranslateDWORDError(hResult) + L'\n';

			m_CMLoggerRef.LogFatal(message);
		}

		m_CMLoggerRef.LogInfo(L"DXContext [Init] | Initialized.\n");

		CD3D11_VIEWPORT viewport(0.0f, 0.0f, (float)clientArea.right - clientArea.left, (float)clientArea.bottom - clientArea.top);
		m_Device.Context()->RSSetViewports(1, &viewport);

		m_Device.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_ShaderLibrary.Init();

		m_Initialized = true;
		m_Shutdown = false;
	}

	void DXContext::Shutdown() noexcept
	{
		if (m_Shutdown)
		{
			m_CMLoggerRef.LogWarning(L"DXContext [Shutdown] | Shutdown has been attempted after shutdown has already occured previously.\n");
			return;
		}

		else if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarning(L"DXContext [Shutdown] | Shutdown has been attempted before initialization.\n");
			return;
		}

		m_Device.Release();
		m_Factory.Release();
		m_SwapChain.Release();
		m_InfoQueue.Release();

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

		if (hResult != S_OK)
		{
			if (!m_InfoQueue.IsQueueEmpty())
				m_InfoQueue.LogMessages();

			std::wstring message = L"DXContext [Present] | Present error : " + 
				WindowsUtility::TranslateDWORDError(hResult) + L'\n';

			m_CMLoggerRef.LogFatal(message);
		}
	}

#pragma endregion
}