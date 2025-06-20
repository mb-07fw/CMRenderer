#pragma once

#include <filesystem>

namespace CMEngine
{
	inline [[nodiscard]] std::filesystem::path CMEAssetMeshDirectory() noexcept
	{
		return std::filesystem::current_path() / "resources\\assets\\meshes\\";
	}
}