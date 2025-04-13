#include "CMR_PCH.hpp"
#include "CMR_DXShaderData.hpp"

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
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& descRef,
		DXImplementedShaderType implementedType
	) noexcept
		: m_VertexData(vertexData), m_PixelData(pixelData),
		  m_Desc(descRef), m_ImplementedType(implementedType)
	{
	}

	void DXShaderSet::CreateShaders(Components::DXDevice& deviceRef, Utility::CMLoggerWide& cmLoggerRef) noexcept
	{
		cmLoggerRef.LogFatalNLIf(m_Created, L"DXShaderSet [CreateShaders] | Attempted to recreate shader set.");

		cmLoggerRef.LogFatalNLIf(mP_VertexShader.Get() != nullptr, L"DXShaderSet [CreateShaders] | Vertex shader interface is already present.");
		cmLoggerRef.LogFatalNLIf(mP_PixelShader.Get() != nullptr, L"DXShaderSet [CreateShaders] | Pixel shader interface is already present.");

		cmLoggerRef.LogFatalNLIf(m_VertexData.pBytecode.Get() == nullptr, L"DXShaderSet [CreateShaders] | Vertex bytecode is nullptr.");
		cmLoggerRef.LogFatalNLIf(m_PixelData.pBytecode.Get() == nullptr, L"DXShaderSet [CreateShaders] | Pixel bytecode is nullptr.");

		HRESULT hResult = deviceRef->CreateVertexShader(m_VertexData.pBytecode->GetBufferPointer(), m_VertexData.pBytecode->GetBufferSize(), nullptr, &mP_VertexShader);

		cmLoggerRef.LogFatalNLIf(hResult != S_OK, L"DXShaderSet [CreateShaders] | Failed to create the vertex shader.");

		hResult = deviceRef->CreatePixelShader(m_PixelData.pBytecode->GetBufferPointer(), m_PixelData.pBytecode->GetBufferSize(), nullptr, &mP_PixelShader);

		cmLoggerRef.LogFatalNLIf(hResult != S_OK, L"DXShaderSet [CreateShaders] | Failed to create the pixel shader.");

		m_Created = true;
	}
}