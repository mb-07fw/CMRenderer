#include "Core/CME_PCH.hpp"
#include "Core/CME_Asset.hpp"

namespace CMEngine::Asset
{
	CMAsset::CMAsset(CMAssetType type) noexcept
		: Type(type)
	{
	}

	CMMesh::CMMesh() noexcept
		: CMAsset(CMAssetType::MESH)
	{
	}

	CMMesh::CMMesh(const CMMeshData& data) noexcept
		: CMAsset(CMAssetType::MESH),
		  Data(data)
	{
	}
}