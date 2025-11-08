#pragma once

#include <cstdint>

namespace CMEngine::ECS
{
	constexpr uint32_t G_ENTITY_INDEX_BITS = 24;
	constexpr uint32_t G_ENTITY_VERSION_BITS = 8;

	/* Shift 1 left by 24 bits :
	 *
	 *		00000000 00000000 00000000 00000001
	 *				to :
	 *		00000001 00000000 00000000 00000000
	 *
	 * Subtract 1 to mask the lower 24 bits :
	 *
	 *	 NOTE : Subtracting 1 in binary means you're flipping the
	 *			lowest 1 bit to 0, and all trailing 0s to 1s until
	 *			the first 1 (carry/borrow effect).
	 *
	 *		00000001 00000000 00000000 00000000
	 *				to :
	 *		00000000 11111111 11111111 11111111
	 */
	constexpr uint32_t G_ENTITY_INDEX_MASK = (1u << G_ENTITY_INDEX_BITS) - 1;

	/* Shift 1 left by 8 bits :
	 *
	 *		00000000 00000000 00000000 00000001
	 *				to :
	 *		00000000 00000000 00000001 00000000
	 *
	 * Subtract 1 to mask the lower 8 bits :
	 *
	 *   NOTE : Subtracting 1 in binary means you're flipping the
	 *			lowest 1 bit to 0, and all trailing 0s to 1s until
	 *			the first 1 (carry/borrow effect).
	 *
	 *		00000000 00000000 00000001 00000000
	 *				to :
	 *		00000000 00000000 00000000 11111111
	 */
	constexpr uint32_t G_ENTITY_VERSION_MASK = (1u << G_ENTITY_VERSION_BITS) - 1;


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

		Entity() = default;
		Entity(uint32_t version, uint32_t index) noexcept;

		uint32_t ToVersion() const noexcept;
		uint32_t ToIndex() const noexcept;

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