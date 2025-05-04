#include "CMC_ECSEntity.hpp"

namespace CMCommon
{
	CMECSEntity::CMECSEntity(uint32_t version, uint32_t index) noexcept
	{
		SetVersion(version);
		SetIndex(index);
	}

	bool CMECSEntity::operator==(CMECSEntity other) const noexcept
	{
		return ID == other.ID;
	}

	CMECSEntity::operator CMECSEntityID() const noexcept
	{
		return ID;
	}

	void CMECSEntity::SetVersion(uint32_t version) noexcept
	{
		// Clear the version bits (top 8), then OR in the shifted new version
		ID = (ID & CMECS_INDEX_MASK) | (version << CMECS_INDEX_BITS);
	}

	void CMECSEntity::SetIndex(uint32_t index) noexcept
	{
		/* Clear the lower 24 bits of ID :
		 *	 (ID & ~CMECS_INDEX_MASK)
		 *
		 * Mask the index to ensure that the input doesn’t overflow into the version bits,
		 *	 as only the lower 24 bits of index are kept, any other bits will be cleared :
		 *
		 *	 (The value of index is clipped to just the lower 24 bits (0x345678), and any
		 *	  bits higher than that (such as 0x12 in the higher byte) are discarded, ensuring
		 *	  no overflow into the version field)
		 *
		 *	 index & CMECS_INDEX_MASK
		 */

		ID = (ID & ~CMECS_INDEX_MASK) | (index & CMECS_INDEX_MASK);
	}

	void CMECSEntity::IncrementVersion() noexcept
	{
		SetVersion(ToVersion() + 1);
	}

	uint8_t CMECSEntity::ToVersion() const noexcept
	{
		// Get the top 8 bits.
		return (ID >> CMECS_INDEX_BITS) & CMECS_VERSION_MASK;
	}

	uint32_t CMECSEntity::ToVersion32() const noexcept
	{
		// Get the top 8 bits.
		return static_cast<uint32_t>((ID >> CMECS_INDEX_BITS) & CMECS_VERSION_MASK);
	}

	uint32_t CMECSEntity::ToIndex() const noexcept
	{
		return ID & CMECS_INDEX_MASK;
	}
}