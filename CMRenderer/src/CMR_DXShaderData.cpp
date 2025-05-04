#include "CMR_PCH.hpp"
#include "CMR_DXShaderData.hpp"

namespace CMRenderer::CMDirectX
{
	DXShaderData::DXShaderData(
		DXShaderSetType implementedType,
		DXShaderType type,
		Microsoft::WRL::ComPtr<ID3DBlob> pShaderBytecode,
		const std::wstring& fileName
	) noexcept
		: CorrespondingShaderSet(implementedType), Type(type),
		  pBytecode(pShaderBytecode), Filename(fileName)
	{
	}
}