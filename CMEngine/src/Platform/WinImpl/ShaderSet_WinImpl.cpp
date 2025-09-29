#include "PCH.hpp"
#include "Platform/WinImpl/ShaderSet_WinImpl.hpp"
#include "Platform/WinImpl/ShaderLibrary_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
#pragma region Shader Sets
	IShaderSet::IShaderSet(ShaderUseID useFlags) noexcept
		: m_UseID(useFlags)
	{
	}

	void IShaderSet::CreateVertexShader(
		const ComPtr<ID3D11Device>& pDevice,
		ComPtr<ID3D11VertexShader>& pOutVertexShader,
		const ShaderRegistry& registry,
		ShaderID id
	) noexcept
	{
		if (pDevice.Get() == nullptr)
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Provided device is nullptr.");

		const ShaderData* pVSData = registry.Retrieve(id);

		if (pVSData == nullptr)
		{
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to retrieve a valid ShaderData from the provided ShaderID.");
			std::exit(-1);
		}

		HRESULT hr = pDevice->CreateVertexShader(
			pVSData->pBytecode->GetBufferPointer(),
			pVSData->pBytecode->GetBufferSize(),
			nullptr,
			&pOutVertexShader
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to create vertex shader.");
	}

	void IShaderSet::CreatePixelShader(
		const ComPtr<ID3D11Device>& pDevice,
		ComPtr<ID3D11PixelShader>& pOutPixelShader,
		const ShaderRegistry& registry,
		ShaderID id
	) noexcept
	{
		if (pDevice.Get() == nullptr)
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Provided device is nullptr.");

		const ShaderData* pPSData = registry.Retrieve(id);

		if (pPSData == nullptr)
		{
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to retrieve a valid ShaderData from the provided ShaderID.");
			std::exit(-1);
		}

		HRESULT hr = pDevice->CreatePixelShader(
			pPSData->pBytecode->GetBufferPointer(),
			pPSData->pBytecode->GetBufferSize(),
			nullptr,
			&pOutPixelShader
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to create pixel shader.");
	}

	void IShaderSet::CreateVertexAndPixelShader(
		const ComPtr<ID3D11Device>& pDevice,
		ComPtr<ID3D11VertexShader>& pOutVertexShader,
		ComPtr<ID3D11PixelShader>& pOutPixelShader,
		const ShaderRegistry& registry,
		ShaderID vertexID,
		ShaderID pixelID
	) noexcept
	{
		CreateVertexShader(pDevice, pOutVertexShader, registry, vertexID);
		CreatePixelShader(pDevice, pOutPixelShader, registry, pixelID);
	}

	void IShaderSet::CreateInputLayout(
		const ComPtr<ID3D11Device>& pDevice,
		ComPtr<ID3D11InputLayout>& pOutInputLayout,
		std::span<const D3D11_INPUT_ELEMENT_DESC> inputDescs,
		const ShaderRegistry& registry,
		ShaderID vertexID
	) noexcept
	{
		if (inputDescs.data() == nullptr)
			spdlog::warn("(WinImpl_IShaderSet) Internal warning: Provided input-element-desc data is nullptr. Input layout creation will likely fail.");
		else if (inputDescs.size() == 0)
			spdlog::warn("(WinImpl_IShaderSet) Internal warning: Provided input-element-desc data is empty. Input layout creation will likely fail.");

		const ShaderData* pVSData = registry.Retrieve(vertexID);

		if (pVSData == nullptr)
		{
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to retrieve a valid ShaderData from the registry with the provided ID.");
			std::exit(-1);
		}

		HRESULT hr = pDevice->CreateInputLayout(
			inputDescs.data(),
			static_cast<UINT>(inputDescs.size()),
			pVSData->pBytecode->GetBufferPointer(),
			pVSData->pBytecode->GetBufferSize(),
			&pOutInputLayout
		);

		if (FAILED(hr))
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Failed to create input layout. Error code: {}", hr);
	}

	void IShaderSet::BindVertexShader(const ComPtr<ID3D11DeviceContext>& pContext, const ComPtr<ID3D11VertexShader>& pVertexShader) noexcept
	{
		if (pContext.Get() == nullptr)
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Provided device context is nullptr.");

		pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
	}

	void IShaderSet::BindPixelShader(const ComPtr<ID3D11DeviceContext>& pContext, const ComPtr<ID3D11PixelShader>& pPixelShader) noexcept
	{
		if (pContext.Get() == nullptr)
			spdlog::critical("(WinImpl_IShaderSet) Internal error: Provided device context is nullptr.");

		pContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
	}

	void IShaderSet::BindInputLayout(const ComPtr<ID3D11DeviceContext>& pContext, const ComPtr<ID3D11InputLayout>& pInputLayout) noexcept
	{
		pContext->IASetInputLayout(pInputLayout.Get());
	}

	void ShaderSetQuad::Create(const ComPtr<ID3D11Device>& pDevice, const ShaderRegistry& registry) noexcept
	{
		CreateVertexAndPixelShader(
			pDevice,
			mP_VertexShader,
			mP_PixelShader,
			registry,
			m_VertexID,
			m_PixelID
		);

		constexpr D3D11_INPUT_ELEMENT_DESC InputLayoutDescs[] = {
			{ "Pos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		CreateInputLayout(
			pDevice,
			mP_InputLayout,
			InputLayoutDescs,
			registry,
			m_VertexID
		);
	}

	void ShaderSetQuad::Bind(const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		BindVertexShader(pContext, mP_VertexShader);
		BindPixelShader(pContext, mP_PixelShader);
		BindInputLayout(pContext, mP_InputLayout);
	}

	void ShaderSetGltf::Create(const ComPtr<ID3D11Device>& pDevice, const ShaderRegistry& registry) noexcept
	{
		CreateVertexAndPixelShader(
			pDevice,
			mP_VertexShader,
			mP_PixelShader,
			registry,
			m_VertexID,
			m_PixelID
		);

		constexpr UINT TexCoordSemanticIndex = 0;
		constexpr D3D11_INPUT_ELEMENT_DESC InputLayoutDescs[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", TexCoordSemanticIndex, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		CreateInputLayout(
			pDevice,
			mP_InputLayout,
			InputLayoutDescs,
			registry,
			m_VertexID
		);
	}

	void ShaderSetGltf::Bind(const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		BindVertexShader(pContext, mP_VertexShader);
		BindPixelShader(pContext, mP_PixelShader);
		BindInputLayout(pContext, mP_InputLayout);
	}
#pragma endregion
}