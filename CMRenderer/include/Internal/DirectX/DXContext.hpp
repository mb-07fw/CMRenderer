#pragma once

#include <minwindef.h>
#include <DirectXMath.h>

#include <string_view>

#include "Core/CMMacros.hpp"
#include "Internal/CMLogger.hpp"
#include "Internal/DirectX/DXComponents.hpp"
#include "Internal/CMShaderLibrary.hpp"

namespace CMRenderer::CMDirectX
{
	struct NormColor {
		float rgba[4];
	};

	class DXContext
	{
	public:
		DXContext(CMLoggerWide& cmLoggerRef) noexcept;
		~DXContext() noexcept;
	public:
		void Init(const HWND hWnd, RECT clientArea) noexcept;
		void Shutdown() noexcept;

		void Clear(NormColor normColor) noexcept;
		void Present() noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }

		template <size_t Elems>
		void Draw(const std::array<float, Elems>& array, UINT vertices) noexcept;

		template <size_t VElems, size_t IElems>
		void DrawIndexed(const std::array<float, VElems>& vertices, const std::array<short, IElems>& indices, float angle) noexcept;
	private:
		void CreateRTV() noexcept;
		void SetViewport(float width, float height) noexcept;
		void SetTopology() noexcept;

		void ReleaseViews() noexcept;
	private:
		CMLoggerWide& m_CMLoggerRef;
		const HWND m_WindowHandle = nullptr;
		Components::DXDevice m_Device;
		Components::DXFactory m_Factory;
		Components::DXSwapChain m_SwapChain;
		CM_IF_NDEBUG_REPLACE(HMODULE m_DebugInterfaceModule = nullptr;)
		CM_IF_NDEBUG_REPLACE(Microsoft::WRL::ComPtr<IDXGIDebug> mP_DebugInterface;)
		CM_IF_NDEBUG_REPLACE(Components::DXInfoQueue m_InfoQueue;)
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mP_RTV;
		CMShaderLibrary m_ShaderLibrary;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};

	template <size_t Elems>
	void DXContext::Draw(const std::array<float, Elems>& array, UINT vertices) noexcept
	{
		m_Device.Context()->OMSetRenderTargets(1, mP_RTV.GetAddressOf(), nullptr);

		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;

		UINT stride = sizeof(float) * 2;
		UINT offset = 0;

		CD3D11_BUFFER_DESC desc(sizeof(float) * Elems, D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA data = {};

		data.pSysMem = array.data();

		HRESULT hResult = m_Device->CreateBuffer(&desc, &data, pVertexBuffer.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the vertex buffer.\n");
		}

		const CMShaderSet& defaultSet = m_ShaderLibrary.GetSetOfType(CMImplementedShaderType::DEFAULT);
		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;

		hResult = m_Device->CreateInputLayout(
			defaultSet.Desc().Data(),
			(UINT)defaultSet.Desc().Size(),
			defaultSet.VertexData().pBytecode->GetBufferPointer(),
			defaultSet.VertexData().pBytecode->GetBufferSize(),
			&pInputLayout
		);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the input layout.\n");
		}

		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
		hResult = m_Device->CreateVertexShader(
			defaultSet.VertexData().pBytecode->GetBufferPointer(),
			defaultSet.VertexData().pBytecode->GetBufferSize(),
			nullptr,
			&pVertexShader
		);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the vertex shader.\n");
		}

		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
		hResult = m_Device->CreatePixelShader(
			defaultSet.PixelData().pBytecode->GetBufferPointer(),
			defaultSet.PixelData().pBytecode->GetBufferSize(),
			nullptr,
			&pPixelShader
		);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the pixel shader.\n");
		}

		m_Device.Context()->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
		m_Device.Context()->IASetInputLayout(pInputLayout.Get());
		m_Device.Context()->VSSetShader(pVertexShader.Get(), nullptr, 0);
		m_Device.Context()->PSSetShader(pPixelShader.Get(), nullptr, 0);

		m_Device.Context()->Draw(vertices, 0);

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_CMLoggerRef.LogFatal(L"DXContext [Draw] | Debug messages generated after drawing.\n");
			}
		);
	}

	template <size_t VElems, size_t IElems>
	void DXContext::DrawIndexed(const std::array<float, VElems>& vertices, const std::array<short, IElems>& indices, float angle) noexcept
	{
		struct CTransform {
			DirectX::XMMATRIX matrix = {};
		};

		m_Device.Context()->OMSetRenderTargets(1, mP_RTV.GetAddressOf(), nullptr);



		const CMShaderSet& shaderSet = m_ShaderLibrary.GetSetOfType(CMImplementedShaderType::POS2D_COLOR_TRANSFORM);
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
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the input layout.\n");
		}



		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
		hResult = m_Device->CreateVertexShader(
			shaderSet.VertexData().pBytecode->GetBufferPointer(),
			shaderSet.VertexData().pBytecode->GetBufferSize(),
			nullptr,
			&pVertexShader
		);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the vertex shader.\n");
		}



		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
		hResult = m_Device->CreatePixelShader(
			shaderSet.PixelData().pBytecode->GetBufferPointer(),
			shaderSet.PixelData().pBytecode->GetBufferSize(),
			nullptr,
			&pPixelShader
		);

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the pixel shader.\n");
		}



		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;

		UINT stride = sizeof(float) * 2;
		UINT offset = 0;



		CD3D11_BUFFER_DESC vDesc(sizeof(float) * VElems, D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA vData = {};
		vData.pSysMem = vertices.data();

		hResult = m_Device->CreateBuffer(&vDesc, &vData, pVertexBuffer.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the vertex buffer.\n");
		}



		CD3D11_BUFFER_DESC iDesc(sizeof(short) * IElems, D3D11_BIND_INDEX_BUFFER);
		D3D11_SUBRESOURCE_DATA iData = {};
		iData.pSysMem = indices.data();

		hResult = m_Device->CreateBuffer(&iDesc, &iData, pIndexBuffer.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the index buffer.\n");
		}



		CTransform cMatrix = {};
		cMatrix.matrix = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixRotationY(angle) *
			DirectX::XMMatrixRotationZ(angle)
		);

		CD3D11_BUFFER_DESC cDesc(sizeof(DirectX::XMMATRIX), D3D11_BIND_CONSTANT_BUFFER);
		D3D11_SUBRESOURCE_DATA cData = {};
		cData.pSysMem = &cMatrix.matrix;

		hResult = m_Device->CreateBuffer(&cDesc, &cData, pConstantBuffer.GetAddressOf());

		if (hResult != S_OK)
		{
			CM_IF_DEBUG(m_InfoQueue.LogMessages());
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the constant buffer.\n");
		}



		m_Device.Context()->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
		m_Device.Context()->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		m_Device.Context()->IASetInputLayout(pInputLayout.Get());

		m_Device.Context()->VSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());

		m_Device.Context()->VSSetShader(pVertexShader.Get(), nullptr, 0);
		m_Device.Context()->PSSetShader(pPixelShader.Get(), nullptr, 0);

		

		m_Device.Context()->DrawIndexed((UINT)indices.size(), 0, 0);

		CM_IF_DEBUG(
			if (!m_InfoQueue.IsQueueEmpty())
			{
				m_InfoQueue.LogMessages();
				m_CMLoggerRef.LogFatal(L"DXContext [Draw] | Debug messages generated after drawing.\n");
			}
		);
	}
}