#pragma once

#include "Core/CME_AssetHandle.hpp"
#include "Core/CME_MeshData.hpp"

namespace CMEngine::Asset
{
	struct CMAsset
	{
		CMAsset(CMAssetType type) noexcept;
		virtual ~CMAsset() = default;

		CMAssetType Type = CMAssetType::INVALID;
	};

	struct CMMesh : public CMAsset
	{
		CMMesh(const CMMeshData& data) noexcept;
		CMMesh() noexcept;
		~CMMesh() = default;

		CMMeshData Data;
	};
}