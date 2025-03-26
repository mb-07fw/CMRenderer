#pragma once

#include <minwindef.h>

#include <string_view>

#include "Internal/CMLogger.hpp"
#include "Internal/DirectX/DXComponents.hpp"
#include "Internal/CMShaderLibrary.hpp"

namespace CMRenderer::DirectX
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
		void Init(const HWND hWnd, const RECT clientArea, const bool isFullscreen) noexcept;
		void Shutdown() noexcept;

		void Clear(NormColor normColor) noexcept;
		void Present() noexcept;

		template <size_t Elems>
		void Draw(const std::array<float, Elems>& array, const UINT vertices) noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }
	private:
		CMLoggerWide& m_CMLoggerRef;
		Components::DXDevice m_Device;
		Components::DXFactory m_Factory;
		Components::DXSwapChain m_SwapChain;
		Components::DXInfoQueue m_InfoQueue;
		CMShaderLibrary m_ShaderLibrary;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mP_RTV;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};

	template <size_t Elems>
	void DXContext::Draw(const std::array<float, Elems>& array, const UINT vertices) noexcept
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
			m_InfoQueue.LogMessages();
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
			m_InfoQueue.LogMessages();
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
			m_InfoQueue.LogMessages();
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
			m_InfoQueue.LogMessages();
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | An error occured when creating the pixel shader.\n");
		}

		m_Device.Context()->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
		m_Device.Context()->IASetInputLayout(pInputLayout.Get());
		m_Device.Context()->VSSetShader(pVertexShader.Get(), nullptr, 0);
		m_Device.Context()->PSSetShader(pPixelShader.Get(), nullptr, 0);

		m_Device.Context()->Draw(vertices, 0);

		if (!m_InfoQueue.IsQueueEmpty())
		{
			m_InfoQueue.LogMessages();
			m_CMLoggerRef.LogFatal(L"DXContext [Draw] | Debug messages generated after drawing.\n");
		}
	}
}