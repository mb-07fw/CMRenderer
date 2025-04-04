#include "Core/CMPCH.hpp"
#include "Internal/Utility/WindowsUtility.hpp"
#include "Internal/DirectX/DXContext.hpp"
#include "Internal/DirectX/DXUtility.hpp"
#include "Internal/DirectX/DXShape.hpp"
#include "Internal/DirectX/DXCamera.hpp"

#define COMMA ,

namespace CMRenderer::CMDirectX
{
#pragma region DXContext
	DXContext::DXContext(Utility::CMLoggerWide& cmLoggerRef, CMWindowData& currentWindowDataRef) noexcept
		: m_CMLoggerRef(cmLoggerRef), m_CurrentWindowDataRef(currentWindowDataRef),
		  m_Device(cmLoggerRef),
		  m_Factory(cmLoggerRef), m_SwapChain(cmLoggerRef),
		  m_ShaderLibrary(cmLoggerRef) CM_IF_NDEBUG_REPLACE(COMMA)
		  CM_IF_NDEBUG_REPLACE(m_InfoQueue(cmLoggerRef))
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

	void DXContext::Init(const HWND hWnd) noexcept
	{
		if (m_Initialized)
		{
			m_CMLoggerRef.LogWarningNL(L"DXContext [Init] | Initializion has been attempted after CMRenderContext has already been initialized.");
			return;
		}

		m_Device.Create();
		m_Factory.Create(m_Device);
		m_SwapChain.Create(hWnd, m_CurrentWindowDataRef.ClientArea, m_Factory, m_Device);

		CM_IF_DEBUG(
			m_InfoQueue.Create(m_Device);
			if (!m_InfoQueue.IsCreated())
			{
				m_CMLoggerRef.LogFatalNL(L"DXContext [Init] | Failed to initialize info queue.");
				return;
			}
		);

		m_ShaderLibrary.Init(m_Device);

		CreateRTV();
		SetViewport();
		SetTopology();

		m_CMLoggerRef.LogInfoNL(L"DXContext [Init] | Initialized.");

		DXCube cube = {};

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

	void DXContext::TestDraw(float rotAngleX, float rotAngleY, float offsetX, float offsetY) noexcept
	{
		m_Device.Context()->OMSetRenderTargets(1, mP_RTV.GetAddressOf(), nullptr);

		RECT& clientAreaRef = m_CurrentWindowDataRef.ClientArea;

		float centerX = (float)clientAreaRef.right / 2;
		float centerY = (float)clientAreaRef.bottom / 2;

		struct Vertex3D {
			struct Pos {
				float x, y, z;
			} pos;
		};

		struct CBTransform {
			DirectX::XMMATRIX transform;
		};

		struct Color {
			float r, g, b, a;
		};

		struct CBColors {
			Color colors[6];
		};

		std::array<Vertex3D, 8> vertices = {{
			{ Vertex3D::Pos{ -1.0f, -1.0f, -1.0f } },
			{ Vertex3D::Pos{  1.0f, -1.0f, -1.0f } },
			{ Vertex3D::Pos{ -1.0f,  1.0f, -1.0f } },
			{ Vertex3D::Pos{  1.0f,  1.0f, -1.0f } },
												  
			{ Vertex3D::Pos{ -1.0f, -1.0f,  1.0f } },
			{ Vertex3D::Pos{  1.0f, -1.0f,  1.0f } },
			{ Vertex3D::Pos{ -1.0f,  1.0f,  1.0f } },
			{ Vertex3D::Pos{  1.0f,  1.0f,  1.0f } },
		}};

		std::array<uint16_t, 36> indices = {
			0, 2, 1,
			2, 3, 1,
			
			1, 3, 5,
			3, 7, 5,

			2, 6, 3,
			3, 6, 7,

			4, 5, 7,
			4, 7, 6,

			0, 4, 2,
			2, 4, 6,
			
			0, 1, 4,
			1, 5, 4
		};

		

		DXShaderSet& shaderSet = m_ShaderLibrary.GetSetOfType(DXImplementedShaderType::DEFAULT3D);

		if (!shaderSet.IsCreated())
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | Shader set wasn't created previously.");

		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;

		HRESULT hResult = m_Device->CreateInputLayout(
			shaderSet.Desc().Data(),
			(UINT)shaderSet.Desc().Size(),
			shaderSet.VertexData().pBytecode->GetBufferPointer(),
			shaderSet.VertexData().pBytecode->GetBufferSize(),
			&pInputLayout
		);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [TestDraw] | An error occured when creating the input layout.\n");
		}

		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pCBTransform;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pCBColors;

		UINT stride = sizeof(DXPos2DInterColorInput);
		UINT offset = 0;

		CD3D11_BUFFER_DESC vDesc((UINT)vertices.size() * sizeof(DXPos2DInterColorInput), D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA vData = {};
		vData.pSysMem = vertices.data();

		hResult = m_Device->CreateBuffer(&vDesc, &vData, pVertexBuffer.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | An error occured when creating the vertex buffer.");
		}

		CD3D11_BUFFER_DESC iDesc(sizeof(uint16_t) * (UINT)indices.size(), D3D11_BIND_INDEX_BUFFER);
		D3D11_SUBRESOURCE_DATA iData = {};
		iData.pSysMem = indices.data();

		hResult = m_Device->CreateBuffer(&iDesc, &iData, pIndexBuffer.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | An error occured when creating the index buffer.");
		}
		
		float aspectRatio = (float)m_CurrentWindowDataRef.ClientArea.right / m_CurrentWindowDataRef.ClientArea.bottom;

		DXCamera camera(0.0f, 0.0f, -15.0f, 45.0f, aspectRatio);

		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(offsetX, offsetY, 0.0f) * 
			DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotAngleX)) * 
			DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotAngleY));

		CBTransform transformBuffer = {};
		transformBuffer.transform = DirectX::XMMatrixTranspose(
			worldMatrix * camera.ViewProjectionMatrix()
		);
		
		CD3D11_BUFFER_DESC cbTransformDesc((UINT)sizeof(CBTransform), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DEFAULT);
		D3D11_SUBRESOURCE_DATA cbTransformData = {};
		cbTransformData.pSysMem = &transformBuffer.transform;

		hResult = m_Device->CreateBuffer(&cbTransformDesc, &cbTransformData, pCBTransform.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | An error occured when creating the transform constant buffer.");
		}

		CBColors colors = {{
			{ 1.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 0.0f, 1.0f }
		}};

		CD3D11_BUFFER_DESC cbColorsDesc((UINT)sizeof(CBColors), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DEFAULT);
		D3D11_SUBRESOURCE_DATA cbColorsData = {};
		cbColorsData.pSysMem = &colors;

		hResult = m_Device->CreateBuffer(&cbColorsDesc, &cbColorsData, pCBColors.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | An error occured when creating the colors constant buffer.");
		}

		m_Device.ContextRaw()->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
		m_Device.ContextRaw()->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		m_Device.ContextRaw()->IASetInputLayout(pInputLayout.Get());

		m_Device.ContextRaw()->VSSetShader(shaderSet.VertexShader(), nullptr, 0);
		m_Device.ContextRaw()->PSSetShader(shaderSet.PixelShader(), nullptr, 0);

		m_Device.ContextRaw()->VSSetConstantBuffers(0, 1, pCBTransform.GetAddressOf());
		m_Device.ContextRaw()->PSSetConstantBuffers(0, 1, pCBColors.GetAddressOf());

		m_Device.ContextRaw()->DrawIndexed((UINT)indices.size(), 0, 0);	

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_CMLoggerRef.LogFatalNL(L"DXContext [TestDraw] | Debug messages generated after drawing.");
			}
		);
	}


	void DXContext::TestTextureDraw() noexcept
	{

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

	void DXContext::SetViewport() noexcept
	{
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, (FLOAT)m_CurrentWindowDataRef.ClientArea.right, (FLOAT)m_CurrentWindowDataRef.ClientArea.bottom);
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