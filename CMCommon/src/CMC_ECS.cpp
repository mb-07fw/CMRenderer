#include "CMC_ECS.hpp"

namespace CMCommon
{
	CMECS::CMECS() noexcept
	{
		m_ReservedEntities.reserve(S_DEFAULT_POOL_SIZE);
		m_DestroyedEntities.reserve(S_DEFAULT_POOL_SIZE);
	}

	[[nodiscard]] CMECSEntity CMECS::CreateEntity() noexcept
	{
		uint32_t index = static_cast<uint32_t>(m_ReservedEntities.size());

		if (m_DestroyedEntities.size() > 0)
		{
			CMECSEntity entity = m_DestroyedEntities.back();
			m_DestroyedEntities.pop_back();

			entity.IncrementVersion();
			m_ReservedEntities.emplace_back(entity);
		}
		else
			m_ReservedEntities.emplace_back(0, index);

		return m_ReservedEntities.back();
	}

	bool CMECS::DestroyEntity(CMECSEntity entity) noexcept
	{
		for (size_t index = 0; index < m_ReservedEntities.size(); ++index)
		{
			CMECSEntity reservedEntity = m_ReservedEntities[index];

			if (reservedEntity != entity)
				continue;

			if (m_ReservedEntities.size() >= 1 || index + 1 != m_ReservedEntities.size())
			{
				CMECSEntity replacingEntity = m_ReservedEntities.back();
				m_ReservedEntities[index] = replacingEntity;
			}

			m_ReservedEntities.pop_back();
			m_DestroyedEntities.emplace_back(reservedEntity);

			DestroyOwnedComponents(entity);

			return true;
		}

		return false;
	}

	[[nodiscard]] bool CMECS::IsEntityCreated(CMECSEntity entity) const noexcept
	{
		for (CMECSEntity reservedEntity : m_ReservedEntities)
			if (entity == reservedEntity)
				return true;

		return false;
	}

	void CMECS::DestroyOwnedComponents(CMECSEntity entity) noexcept
	{
		for (const auto& [typeID, sparseSet] : m_MappedComponentSets)
			if (sparseSet->ContainsEntity(entity))
				sparseSet->RemoveEntity(entity);
	}
}