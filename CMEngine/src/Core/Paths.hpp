#pragma once

#include <filesystem>

#ifndef CM_SOURCE_DIR
	#error CM_SOURCE_DIR is not defined. This prevents inference of relevant project directories and will cause the build to not run correctly.
#endif

namespace CMEngine::Core
{
	inline [[nodiscard]] std::filesystem::path RootDirectory() noexcept
	{
		return CM_SOURCE_DIR;
	}

	inline [[nodiscard]] std::filesystem::path OutDirectory() noexcept
	{
		std::filesystem::path sourcePath = CM_SOURCE_DIR;

		return sourcePath / "build" / CM_CONFIG / "CMEngine" / "out";
	}

	inline [[nodiscard]] std::filesystem::path AssetMeshDirectory() noexcept
	{
		return RootDirectory() / "CMEngine" / "resources\\assets\\meshes\\";
	}

	inline [[nodiscard]] std::filesystem::path AssetShaderDirectory() noexcept
	{
		return RootDirectory() / "CMEngine" / "resources" / "shaders";
	}
}