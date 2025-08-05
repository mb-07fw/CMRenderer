#pragma once

#include "CMPlatform/IPlatform.hpp"

#include <memory>

namespace CMEngine::Platform
{
	using PlatformInFunc = void (*)(IPlatform**);
	using PlatformOutFunc = IPlatform* (*)();

	using PlatformPointerRaw = IPlatform*;

#ifdef _WIN32
	[[nodiscard]] void Win32LoadPlatform(IPlatform*& pPlatform) noexcept;
#endif

	/* Loads the interface of the current platform. */
	[[nodiscard]] void LoadInterface(IPlatform*& pPlatform) noexcept;

	/* Unloads the interface of the current platform. */
	void Unload(IPlatform*& pPlatform) noexcept;
}