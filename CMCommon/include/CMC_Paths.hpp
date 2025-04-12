#pragma once

#include <filesystem>

namespace CMRenderer::Utility
{
	inline [[nodiscard]] std::filesystem::path RendererOutDirectoryRelativeToEngine() noexcept
	{
		std::filesystem::path workingDirectory = std::filesystem::current_path();

		std::filesystem::path rendererOutDirectory = workingDirectory.parent_path() / "build" / "CMRenderer" / CM_CONFIG / "Out";

		return rendererOutDirectory;
	}
}