#include "Core/PCH.hpp"
#include "DX/DX11/DX11_ShaderData.hpp"

namespace CMEngine::DX::DX11
{
	ShaderData::ShaderData(
		ShaderSetType implementedType,
		ShaderType type,
		Microsoft::WRL::ComPtr<ID3DBlob> pShaderBytecode,
		const std::wstring& fileName
	) noexcept
		: CorrespondingShaderSet(implementedType), Type(type),
		  pBytecode(pShaderBytecode), Filename(fileName)
	{
	}
}