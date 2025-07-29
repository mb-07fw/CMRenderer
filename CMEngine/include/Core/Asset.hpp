#pragma once

#include "Core/AssetHandle.hpp"
#include "Core/MeshData.hpp"

namespace CMEngine::Core::Asset
{
	struct Asset
	{
		Asset(AssetType type) noexcept;
		virtual ~Asset() = default;

		AssetType Type = AssetType::INVALID;
	};

	struct Mesh : public Asset
	{
		Mesh(const MeshData& data) noexcept;
		Mesh() noexcept;
		~Mesh() = default;

		MeshData Data;
	};
}