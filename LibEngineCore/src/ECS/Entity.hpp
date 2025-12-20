#pragma once

#include <cstdint>

#include "Types.hpp"

namespace CMEngine::ECS
{
	inline constexpr uint32_t G_Entity_Index_Bits = 24;
	inline constexpr uint32_t G_Entity_Version_Bits = 8;
	inline constexpr uint32_t G_Entity_Index_Shift = 0; /* Start at first bit... */
	inline constexpr uint32_t G_Entity_Version_Shift = G_Entity_Index_Bits; /* Version field starts at the end of index field... */

	using IndexField = Bitfield<uint32_t, G_Entity_Index_Bits, G_Entity_Index_Shift>;
	using VersionField = Bitfield<uint32_t, G_Entity_Version_Bits, G_Entity_Version_Shift>;

	/* 31         24 23                        0
	 *	+----------+---------------------------+
	 *	| VERSION  | INDEX					   |
	 *	+----------+---------------------------+
	 *	8 bits         24 bits
	 */
	using EntityID = uint32_t;

	struct Entity
	{
		friend class ECS;
		friend class ArchetypeECS;

		Entity() = default;
		~Entity() = default;

		Entity(uint32_t version, uint32_t index) noexcept;

		uint32_t Version() const noexcept;
		uint32_t Index() const noexcept;

		[[nodiscard]] bool operator==(Entity other) const noexcept;
		[[nodiscard]] operator EntityID() const noexcept;

		Entity& operator=(EntityID id) noexcept;
	private:
		void SetVersion(uint32_t version) noexcept;
		void SetIndex(uint32_t index) noexcept;

		void IncrementVersion() noexcept;

		EntityID ID = 0;
	};
}