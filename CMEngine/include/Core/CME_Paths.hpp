#pragma once

#include <filesystem>

namespace CMEngine
{
	inline [[nodiscard]] std::filesystem::path CMERootDirectory() noexcept
	{
		#ifdef CM_CMAKE
			return std::filesystem::current_path() / "..\\..\\..\\..\\";
		#else
			return std::filesystem::current_path();
		#endif
	}

	inline [[nodiscard]] std::filesystem::path CMEOutDirectory() noexcept
	{
		#ifdef CM_CMAKE
			return std::filesystem::current_path();
		#else
			return std::filesystem::current_path() / "build" / CM_CONFIG / "CMEngine" / "out";
		#endif
	}

	inline [[nodiscard]] std::filesystem::path CMEAssetMeshDirectory() noexcept
	{
		return CMERootDirectory() / "CMEngine" / "resources\\assets\\meshes\\";
	}
}