#pragma once

#include <cstdint>

namespace CMCommon
{
	constexpr uint32_t CMECS_INDEX_BITS = 24;
	constexpr uint32_t CMECS_VERSION_BITS = 8;

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
	constexpr uint32_t CMECS_INDEX_MASK = (1u << CMECS_INDEX_BITS) - 1;

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
	constexpr uint32_t CMECS_VERSION_MASK = (1u << CMECS_VERSION_BITS) - 1;

	using CMECSEntityID = uint32_t;

	struct CMECSEntity
	{
		friend class CMECS;

		CMECSEntity() = default;
		CMECSEntity(uint32_t version, uint32_t index) noexcept;

		uint8_t ToVersion() const noexcept;
		// Note to self -- If trying to print the version, use ToVersion32, or the version will be printed as an unsigned char, which is a uint8_t.
		uint32_t ToVersion32() const noexcept;
		uint32_t ToIndex() const noexcept;
	private:
		bool operator==(CMECSEntity other) const noexcept;
		operator CMECSEntityID() const noexcept;

		void SetVersion(uint32_t version) noexcept;
		void SetIndex(uint32_t index) noexcept;

		void IncrementVersion() noexcept;

		CMECSEntityID ID = 0u;
	};
}