#pragma once

#include "Platform/Core/InputLayout.hpp"

#include <cstdint>
#include <array>

namespace CMEngine::Pass
{
	enum class Permutation : int8_t
	{
		Invalid = -1,
		Gltf_Basic,
		Count
	};
}