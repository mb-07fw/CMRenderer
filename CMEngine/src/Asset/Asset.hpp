#pragma once

#include "Asset/AssetID.hpp"
#include "Types.hpp"

#include <cstdint>
#include <vector>

namespace CMEngine::Asset
{
	struct Vertex
	{
		Float3 Pos;
		Float3 Normal;
		Float3 TexCoord;
	};

	using Index = uint16_t;

	struct Material
	{
		/* NOTE: Semantically, it is completely valid for a mesh to not have a texture,
		 *		   since by default AssetID's are invalid. */
		AssetID TextureID;
		Color4 BaseColor;
		float Metallic = 0.0f;
		float Roughness = 0.0f;
	};

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<Index> Indices;
		Material Material;
	};

	struct Mesh : public Asset
	{
		inline Mesh(const MeshData& data) noexcept
			: Asset(AssetType::MESH),
			Data(data)
		{
		}

		inline Mesh() noexcept
			: Asset(AssetType::MESH)
		{
		}

		~Mesh() = default;

		MeshData Data;
	};
}