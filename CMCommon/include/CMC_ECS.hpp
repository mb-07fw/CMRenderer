#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>

#include "CMC_ECSEntity.hpp"
#include "CMC_SparseSet.hpp"

namespace CMCommon
{
	class CMECS
	{
	public:
		CMECS() noexcept;
		~CMECS() = default;
	public:
		/* Creates an stores a reserved CMECSEntity. */
		[[nodiscard]] CMECSEntity CreateEntity() noexcept;

		/* Invalidates and recycles an entity as well as it's components if any were present.
		 * Returns true if the entity was destroyed, false otherwise. */
		bool DestroyEntity(CMECSEntity entity) noexcept;

		/* Emplaces a component into a sparse set that corresponds to ComponentTy's CMECSTypeID, provided by CMECSTypeWrangler.
		 * A component is not created if the entity is invalid. 
		 * Returns true if the component was successfully emplaced, false otherwise.
		 * Returns false if the entity is invalid, either due to being destroyed or not being registered. */
		template <typename ComponentTy, typename... Args>
		inline bool EmplaceComponent(CMECSEntity entity, Args&&... args) noexcept;

		/* Queries the ComponentTy's corresponding ICMSparseSet if it contains the entity ID.
		 * Returns true if the component was registered to the component, false otherwise.
		 * Returns false if the entity is invalid, either due to being destroyed or not being registered. */
		template <typename ComponentTy>
		inline [[nodiscard]] bool HasComponent(CMECSEntity entity) const noexcept;

		/* Returns a raw pointer to a component type from a tied entity. 
		 * May return nullptr if the entity is invalid, or the component doesn't exist.
		 * Note that if the addition of a component, or destruction of a stale component causes a resize of the component vector, any pointers
		 * retrieved before the resize will become dangling due to the semantics of std::vector. To combat this, copy the component data when
		 * ownership is needed, or refrain from persisting ComponentTy pointers across instances where components are added.
		 * A pointer is returned over a reference due to the possibility of misuse where a valid reference to a component may not exist. */
		template <typename ComponentTy>
		inline [[nodiscard]] ComponentTy* GetComponent(CMECSEntity entity) noexcept;
	private:
		/* Iterates through all entities in m_ReservedEntities and compares the @entity's ID to each reserved entity's ID.
		 * Returns true if a matching entity ID was found, false otherwise.
		 * TODO : Replace linear searching with a bitset or hash set for faster searching. */
		[[nodiscard]] bool IsEntityCreated(CMECSEntity entity) const noexcept;

		/* Destroys all components owned by the entity. Returns if the entity is invalid, or no component's were tied to the entity. */
		void DestroyOwnedComponents(CMECSEntity entity) noexcept;
	private:
		static constexpr size_t S_DEFAULT_POOL_SIZE = 20;
		std::vector<CMECSEntity> m_ReservedEntities;
		std::vector<CMECSEntity> m_DestroyedEntities;
		std::unordered_map<CMECSTypeID, std::unique_ptr<ICMSparseSet>> m_MappedComponentSets;
	};

	template <typename ComponentTy, typename... Args>
	bool CMECS::EmplaceComponent(CMECSEntity entity, Args&&... args) noexcept
	{
		if (!IsEntityCreated(entity))
			return false;

		CMECSTypeID componentTypeID = CMECSTypeWrangler::GetTypeID<ComponentTy>();

		auto it = m_MappedComponentSets.find(componentTypeID);

		if (it == m_MappedComponentSets.end())
			m_MappedComponentSets[componentTypeID] = std::make_unique<CMSparseSet<ComponentTy>>();

		std::unique_ptr<ICMSparseSet>& ptrRef = m_MappedComponentSets[componentTypeID];

		// Sanity check...
		if (componentTypeID != ptrRef->TypeID())
			return false;

		CMSparseSet<ComponentTy>* pSparseSet = static_cast<CMSparseSet<ComponentTy>*>(ptrRef.get());

		if (pSparseSet->ContainsEntity(entity))
			return false;

		pSparseSet->EmplaceComponent(entity, std::forward<Args>(args)...);

		return true;
	}

	template <typename ComponentTy>
	inline [[nodiscard]] bool CMECS::HasComponent(CMECSEntity entity) const noexcept
	{
		if (!IsEntityCreated(entity))
			return false;

		CMECSTypeID componentTypeID = CMECSTypeWrangler::GetTypeID<ComponentTy>();

		auto it = m_MappedComponentSets.find(componentTypeID);

		if (it == m_MappedComponentSets.end())
			return false;

		const std::unique_ptr<ICMSparseSet>& ptrRef = it->second;

		const CMSparseSet<ComponentTy>* pSparseSet = static_cast<const CMSparseSet<ComponentTy>*>(ptrRef.get());

		return pSparseSet->ContainsEntity(entity);
	}

	template <typename ComponentTy>
	[[nodiscard]] ComponentTy* CMECS::GetComponent(CMECSEntity entity) noexcept
	{
		if (!IsEntityCreated(entity))
			return nullptr;

		CMECSTypeID componentTypeID = CMECSTypeWrangler::GetTypeID<ComponentTy>();

		auto it = m_MappedComponentSets.find(componentTypeID);

		if (it == m_MappedComponentSets.end())
			return nullptr;

		return static_cast<CMSparseSet<ComponentTy>*>(it->second.get())->GetComponent(entity);
	}
}