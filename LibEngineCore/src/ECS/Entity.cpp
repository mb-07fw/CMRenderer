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
		VersionField::Insert(version, ID);
	}

	void Entity::SetIndex(uint32_t index) noexcept
	{
		IndexField::Insert(index, ID);
	}

	void Entity::IncrementVersion() noexcept
	{
		SetVersion(Version() + 1);
	}

	uint32_t Entity::Version() const noexcept
	{
		return VersionField::Extract(ID);
	}

	uint32_t Entity::Index() const noexcept
	{
		return IndexField::Extract(ID);
	}
}