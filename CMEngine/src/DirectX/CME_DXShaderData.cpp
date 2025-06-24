#include "Core/CME_PCH.hpp"
#include "DirectX/CME_DXShaderData.hpp"

namespace CMEngine::DirectXAPI::DX11
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