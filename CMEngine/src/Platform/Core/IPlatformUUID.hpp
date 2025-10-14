#pragma once

#include "Export.hpp"

#include <functional>
#include <cstdint>
#include <cstring>

namespace CMEngine
{
	struct CM_ENGINE_API UUID
	{
		static constexpr size_t S_NUM_BYTES = 16;

		unsigned char ID[S_NUM_BYTES] =
		{
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		inline bool operator==(const UUID& other) const noexcept
		{
			return std::memcmp(ID, other.ID, S_NUM_BYTES) == 0;
		}
	};
}

namespace std
{
    template <>
    struct hash <CMEngine::UUID>
    {
        inline std::size_t operator()(const CMEngine::UUID& uuid) const noexcept
        {
            static_assert(sizeof(std::size_t) == 8 || sizeof(std::size_t) == 4);

            std::size_t hash = 0;
            std::size_t part1, part2;
            std::memcpy(&part1, uuid.ID + 0, 8);
            std::memcpy(&part2, uuid.ID + 8, 8);

            constexpr std::size_t fnv_offset = sizeof(std::size_t) == 8 ? 14695981039346656037ull : 2166136261u;
            constexpr std::size_t fnv_prime = sizeof(std::size_t) == 8 ? 1099511628211ull : 16777619u;

            hash = fnv_offset;
            hash ^= part1; hash *= fnv_prime;
            hash ^= part2; hash *= fnv_prime;

            return hash;
        }
    };
}