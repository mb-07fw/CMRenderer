#pragma once

namespace CMEngine
{
	inline constexpr void HashCombine(size_t& outSeed, size_t value) noexcept
	{
		constexpr size_t UnholyMagicConstantIDontUnderstandThanksChatGPT = 0x9e3779b97f4a7c15ull; // aka 2^64...

		outSeed ^= value + UnholyMagicConstantIDontUnderstandThanksChatGPT + (outSeed << 6) + (outSeed >> 2);
	}
}