#include "Core/PCH.hpp"
#include "Core/Asset.hpp"

namespace CMEngine::Core::Asset
{
	Asset::Asset(AssetType type) noexcept
		: Type(type)
	{
	}

	Mesh::Mesh() noexcept
		: Asset(AssetType::MESH)
	{
	}

	Mesh::Mesh(const MeshData& data) noexcept
		: Asset(AssetType::MESH),
		  Data(data)
	{
	}
}