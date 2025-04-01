#include "Core/CMPCH.hpp"
#include "Internal/DirectX/DXShaderData.hpp"

namespace CMRenderer::CMDirectX
{
	CMShaderData::CMShaderData(
		CMImplementedShaderType implementedType,
		CMShaderType type,
		Microsoft::WRL::ComPtr<ID3DBlob> pShaderBytecode,
		const std::wstring& fileName
	) noexcept
		: ImplementedType(implementedType), Type(type),
		pBytecode(pShaderBytecode), Filename(fileName)
	{
	}

	CMShaderSet::CMShaderSet(
		const CMShaderData& vertexData,
		const CMShaderData& pixelData,
		const Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC>& descRef,
		CMImplementedShaderType implementedType
	) noexcept
		: m_VertexData(vertexData), m_PixelData(pixelData),
		  m_Desc(descRef), m_ImplementedType(implementedType)
	{
	}
}