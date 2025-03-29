#include "Core/CMPCH.hpp"
#include "Internal/WindowsUtility.hpp"
#include "Internal/DirectX/DXContext.hpp"
#include "Internal/DirectX/DXUtility.hpp"

#define APPEND_COMMA(x) x,

namespace CMRenderer::DirectX
{
#pragma region DXContext
	DXContext::DXContext(CMLoggerWide& cmLoggerRef) noexcept
		: m_CMLoggerRef(cmLoggerRef), m_Device(cmLoggerRef),
		  m_Factory(cmLoggerRef), m_SwapChain(cmLoggerRef),
		  CM_IF_NDEBUG_REPLACE(APPEND_COMMA(m_InfoQueue(cmLoggerRef)))
		  m_ShaderLibrary(cmLoggerRef)
	{
	}

	DXContext::~DXContext() noexcept
	{
		if (m_Initialized)
			Shutdown();

		// Load Dxgidebug.dll dynamically
		HMODULE dxgiDebugModule = LoadLibrary(L"Dxgidebug.dll");

		if (dxgiDebugModule == nullptr)
			m_CMLoggerRef.LogWarning(L"DXContext [Shutdown] | Failed to load Dxgidebug.dll\n");
		else
		{
			typedef HRESULT(WINAPI* DXGIGetDebugInterfaceFunc)(const IID&, void**);

			// Retrieve the address of DXGIGetDebugInterface function
			DXGIGetDebugInterfaceFunc pDXGIGetDebugInterface =
				(DXGIGetDebugInterfaceFunc)GetProcAddress(dxgiDebugModule, "DXGIGetDebugInterface");

			if (!pDXGIGetDebugInterface)
				m_CMLoggerRef.LogWarning(L"DXContext [Shutdown] | Failed to get function address for DXGIGetDebugInterface\n");
			else
			{
				Microsoft::WRL::ComPtr<IDXGIDebug> pDebug;
				HRESULT hResult = pDXGIGetDebugInterface(IID_PPV_ARGS(&pDebug));

				if (hResult != S_OK)
					m_CMLoggerRef.LogWarning(L"DXContext [Shutdown] | Failed to retrieve a DXGI debug interface.\n");
				else
				{
					hResult = pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);

					if (hResult != S_OK)
						m_CMLoggerRef.LogWarning(L"DXContext [Shutdown] | Failed to report live objects.\n");
				}
			}

			FreeLibrary(dxgiDebugModule);
		}
	}

	void DXContext::Init(const HWND hWnd, RECT clientArea, bool isFullscreen) noexcept
	{
		if (m_Initialized)
		{
			m_CMLoggerRef.LogWarning(L"DXContext [Init] | Initializion has been attempted after CMRenderContext has already been initialized.\n");
			return;
		}

		m_Fullscreen = isFullscreen;

		m_Device.Create();
		m_Factory.Create(m_Device);
		m_SwapChain.Create(hWnd, clientArea, m_Factory, m_Device, m_Fullscreen);

		CM_IF_DEBUG(
			m_InfoQueue.Create(m_Device);
			if (!m_InfoQueue.IsCreated())
			{
				m_CMLoggerRef.LogFatal(L"DXContext [Init] | Failed to initialize info queue.\n");
				return;
			}
		);

		m_ShaderLibrary.Init();

		CreateRTV();
		SetViewport((float)clientArea.right, (float)clientArea.bottom);
		SetTopology();

		m_CMLoggerRef.LogInfo(L"DXContext [Init] | Initialized.\n");

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
		
		HRESULT hResult = m_SwapChain->SetFullscreenState(false, nullptr);

		if (hResult != S_OK)
			m_CMLoggerRef.LogWarning(L"DXContext [Shutdown] | Failed to reset fullscreen state.\n");

		mP_RTV.Reset();
		m_Factory.Release();
		m_Device.Release();
		m_SwapChain.Release();

		if (!m_InfoQueue.IsQueueEmpty())
			m_InfoQueue.LogMessages();

		CM_IF_DEBUG(m_InfoQueue.Release());
			
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

			std::wstring message = L"DXContext [Present] | Device removed error : " +
				WindowsUtility::TranslateDWORDError(hResult) + L'\n';

			m_CMLoggerRef.LogFatal(message);
			return;
		}
		else if (hResult != S_OK)
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
			);

			std::wstring message = L"DXContext [Present] | Present error : " + 
				WindowsUtility::TranslateDWORDError(hResult) + L'\n';

			m_CMLoggerRef.LogFatal(message);
			return;
		}
	}

	void DXContext::ToggleFullscreen(bool state) noexcept
	{
		if (!m_Initialized)
		{
			m_CMLoggerRef.LogWarning(L"DXContext [ToggleFullscreen] | Attempted to set context state before initialization.\n");
			return;
		}

		if (m_Fullscreen == state)
			return;

		mP_RTV.Reset();
		HRESULT hResult = m_SwapChain->SetFullscreenState(state, nullptr);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
			);

			m_CMLoggerRef.LogFatal(L"DXContext [ToggleFullscreen] | Failed to set fullscreen state.\n");
			return;
		}

		DXGI_SWAP_CHAIN_DESC previousDesc;
		hResult = m_SwapChain->GetDesc(&previousDesc);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
					);

			m_CMLoggerRef.LogFatal(L"DXContext [ToggleFullscreen] | Failed to get previous swap chain desc.\n");
			return;
		}

		hResult = m_SwapChain->ResizeBuffers(previousDesc.BufferCount, 0, 0, DXGI_FORMAT_UNKNOWN, previousDesc.Flags);

		if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
		{
			m_CMLoggerRef.LogFatal(L"DXContext [ToggleFullscreen] | Failed to resize buffers due to device loss.\n");
			return;
		}
		else if (hResult != S_OK)
		{
			m_CMLoggerRef.LogFatal(L"DXContext [ToggleFullscreen] | Failed to resize buffers.\n");
			return;
		}

		DXGI_SWAP_CHAIN_DESC currentDesc;
		hResult = m_SwapChain->GetDesc(&currentDesc);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(
				if (!m_InfoQueue.IsQueueEmpty())
					m_InfoQueue.LogMessages();
					);

			m_CMLoggerRef.LogFatal(L"DXContext [ToggleFullscreen] | Failed to get current swap chain desc.\n");
			return;
		}

		SetViewport((float)currentDesc.BufferDesc.Width, (float)currentDesc.BufferDesc.Height);
		CreateRTV();

		m_Fullscreen = state;
	}

	/*void DXContext::ResizeTo(RECT newClientArea) noexcept
	{
		mP_RTV.Reset();
		m_Device.Context()->ClearState();

		DXGI_SWAP_CHAIN_DESC currentDesc;
		m_SwapChain->GetDesc(&currentDesc);

		m_SwapChain->ResizeBuffers(
			currentDesc.BufferCount,
			newClientArea.right - newClientArea.left, 
			newClientArea.bottom - newClientArea.top,
			DXGI_FORMAT_UNKNOWN, 
			currentDesc.Flags
		);

		CreateRTV();
		SetViewport(newClientArea);
		SetTopology();
	}*/

	void DXContext::CreateRTV() noexcept
	{
		// Reset RTV in case it was created previously. Failure to do so will result in a memory leak.
		mP_RTV.Reset();

		// Get the back buffer.
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		HRESULT hResult = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		if (hResult != S_OK)
		{
			std::wstring message = L"DXContext [CreateRTV] | Failed to get back buffer : " +
				WindowsUtility::TranslateDWORDError(hResult) + L'\n';

			m_CMLoggerRef.LogFatal(message);
			return;
		}

		// Create the RTV.
		hResult = m_Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, mP_RTV.GetAddressOf());

		if (hResult != S_OK)
		{
			std::wstring message = L"DXContext [CreateRTV] | Failed to create the Render Target View : " +
				WindowsUtility::TranslateDWORDError(hResult) + L'\n';

			m_CMLoggerRef.LogFatal(message);
		}
	}

	void DXContext::SetViewport(float width, float height) noexcept
	{
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, width, height);
		m_Device.Context()->RSSetViewports(1, &viewport);
	}

	void DXContext::SetTopology() noexcept
	{
		m_Device.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
#pragma endregion
}