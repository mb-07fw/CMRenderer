#include "Core/PCH.hpp"
#include "Core/ECSEntity.hpp"

namespace CMEngine::Core
{
	ECSEntity::ECSEntity(uint32_t version, uint32_t index) noexcept
	{
		SetVersion(version);
		SetIndex(index);
	}

	ECSEntity& ECSEntity::operator=(ECSEntityID id) noexcept
	{
		ID = id;
		return *this;
	}

	bool ECSEntity::operator==(ECSEntity other) const noexcept
	{
		return ID == other.ID;
	}

	ECSEntity::operator ECSEntityID() const noexcept
	{
		return ID;
	}

	void ECSEntity::SetVersion(uint32_t version) noexcept
	{
		// Clear the version bits (top 8), then OR in the shifted new version
		ID = (ID & G_ECS_INDEX_MASK) | (version << G_ECS_INDEX_BITS);
	}

	void ECSEntity::SetIndex(uint32_t index) noexcept
	{
		/* Clear the lower 24 bits of ID :
		 *	 (ID & ~G_ECS_INDEX_MASK)
		 *
		 * Mask the index to ensure that the input doesn't overflow into the version bits,
		 *	 as only the lower 24 bits of index are kept, any other bits will be cleared :
		 *
		 *	 (The value of index is clipped to just the lower 24 bits (0x345678), and any
		 *	  bits higher than that (such as 0x12 in the higher byte) are discarded, ensuring
		 *	  no overflow into the version field)
		 *
		 *	 index & G_ECS_INDEX_MASK
		 */

		ID = (ID & ~G_ECS_INDEX_MASK) | (index & G_ECS_INDEX_MASK);
	}

	void ECSEntity::IncrementVersion() noexcept
	{
		SetVersion(ToVersion() + 1);
	}

	uint32_t ECSEntity::ToVersion() const noexcept
	{
		// Get the top 8 bits.
		return static_cast<uint32_t>((ID >> G_ECS_INDEX_BITS) & G_ECS_VERSION_MASK);
	}

	uint32_t ECSEntity::ToIndex() const noexcept
	{
		return ID & G_ECS_INDEX_MASK;
	}
}