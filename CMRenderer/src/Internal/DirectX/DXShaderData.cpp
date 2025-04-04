#include "Core/CMPCH.hpp"
#include "Internal/DirectX/DXShaderData.hpp"

namespace CMRenderer::CMDirectX
{
	DXShaderData::DXShaderData(
		DXImplementedShaderType implementedType,
		DXShaderType type,
		Microsoft::WRL::ComPtr<ID3DBlob> pShaderBytecode,
		const std::wstring& fileName
	) noexcept
		: ImplementedType(implementedType), Type(type),
		pBytecode(pShaderBytecode), Filename(fileName)
	{
	}

	DXShaderSet::DXShaderSet(
		const DXShaderData& vertexData,
		const DXShaderData& pixelData,
		const Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC>& descRef,
		DXImplementedShaderType implementedType
	) noexcept
		: m_VertexData(vertexData), m_PixelData(pixelData),
		  m_Desc(descRef), m_ImplementedType(implementedType)
	{
	}

	void DXShaderSet::CreateShaders(Components::DXDevice& deviceRef, Utility::CMLoggerWide& cmLoggerRef) noexcept
	{
		if (m_Created)
			cmLoggerRef.LogFatalNL(L"DXShaderSet [CreateShaders] | Attempted to recreate shader set.");

		if (mP_VertexShader.Get() != nullptr)
			cmLoggerRef.LogFatalNL(L"DXShaderSet [CreateShaders] | Vertex shader inteface is already present.");
		else if (mP_PixelShader.Get() != nullptr)
			cmLoggerRef.LogFatalNL(L"DXShaderSet [CreateShaders] | Pixel shader interface is already present.");

		if (m_VertexData.pBytecode.Get() == nullptr)
			cmLoggerRef.LogFatalNL(L"DXShaderSet [CreateShaders] | Vertex bytecode is nullptr.");
		else if (m_PixelData.pBytecode.Get() == nullptr)
			cmLoggerRef.LogFatalNL(L"DXShaderSet [CreateShaders] | Pixel bytecode is nullptr.");

		HRESULT hResult = deviceRef->CreateVertexShader(m_VertexData.pBytecode->GetBufferPointer(), m_VertexData.pBytecode->GetBufferSize(), nullptr, &mP_VertexShader);

		if (hResult != S_OK)
			cmLoggerRef.LogFatalNL(L"DXShaderSet [CreateShaders] | Failed to create the vertex shader.");

		hResult = deviceRef->CreatePixelShader(m_PixelData.pBytecode->GetBufferPointer(), m_PixelData.pBytecode->GetBufferSize(), nullptr, &mP_PixelShader);

		if (hResult != S_OK)
			cmLoggerRef.LogFatalNL(L"DXShaderSet [CreateShaders] | Failed to create the pixel shader.");

		m_Created = true;
	}
}