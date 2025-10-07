#include "PCH.hpp"
#include "Entity.hpp"

namespace CMEngine::ECS
{
	Entity::Entity(uint32_t version, uint32_t index) noexcept
	{
		SetVersion(version);
		SetIndex(index);
	}

	Entity& Entity::operator=(EntityID id) noexcept
	{
		ID = id;
		return *this;
	}

	bool Entity::operator==(Entity other) const noexcept
	{
		return ID == other.ID;
	}

	Entity::operator EntityID() const noexcept
	{
		return ID;
	}

	void Entity::SetVersion(uint32_t version) noexcept
	{
		// Clear the version bits (top 8), then OR in the shifted new version
		ID = (ID & G_ENTITY_INDEX_MASK) | (version << G_ENTITY_INDEX_BITS);
	}

	void Entity::SetIndex(uint32_t index) noexcept
	{
		/* Clear the lower 24 bits of ID :
		 *	 (ID & ~G_ENTITY_INDEX_MASK)
		 *
		 * Mask the index to ensure that the input doesn't overflow into the version bits,
		 *	 as only the lower 24 bits of index are kept, any other bits will be cleared :
		 *
		 *	 (The value of index is clipped to just the lower 24 bits (0x345678), and any
		 *	  bits higher than that (such as 0x12 in the higher byte) are discarded, ensuring
		 *	  no overflow into the version field)
		 *
		 *	 index & G_ENTITY_INDEX_MASK
		 */

		ID = (ID & ~G_ENTITY_INDEX_MASK) | (index & G_ENTITY_INDEX_MASK);
	}

	void Entity::IncrementVersion() noexcept
	{
		SetVersion(ToVersion() + 1);
	}

	uint32_t Entity::ToVersion() const noexcept
	{
		// Get the top 8 bits.
		return static_cast<uint32_t>((ID >> G_ENTITY_INDEX_BITS) & G_ENTITY_VERSION_MASK);
	}

	uint32_t Entity::ToIndex() const noexcept
	{
		return ID & G_ENTITY_INDEX_MASK;
	}
}