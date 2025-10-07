#pragma once

#include "Entity.hpp"
#include "SparseSet.hpp"
#include "Macros.hpp"

#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace std
{
	template <>
	struct hash<CMEngine::ECS::TypeID>
	{
		inline size_t operator()(CMEngine::ECS::TypeID typeID) const noexcept
		{
			return hash<size_t>{}(typeID.ID);
		}
	};
}

namespace CMEngine::ECS
{
	class ECS
	{
	public:
		ECS() noexcept;
		~ECS() = default;
	public:
		/* Creates an stores a reserved Entity. */
		[[nodiscard]] Entity CreateEntity() noexcept;

		/* Invalidates and recycles an entity as well as it's components if any were present.
		 * Returns true if the entity was destroyed, false otherwise. */
		bool DestroyEntity(Entity entity) noexcept;

		/* Emplaces a component into a sparse set that corresponds to ComponentTy's TypeID, provided by TypeWrangler.
		 * A component is not created if the entity is invalid.
		 * Returns true if the component was successfully emplaced, false otherwise.
		 * Returns false if the entity is invalid, either due to being destroyed or not being registered. */
		template <typename ComponentTy, typename... Args>
		inline bool EmplaceComponent(Entity entity, Args&&... args) noexcept;

		/* Queries the ComponentTy's corresponding ISparseSet if it contains the entity ID.
		 * Returns true if the component was registered to the component, false otherwise.
		 * Returns false if the entity is invalid, either due to being destroyed or not being registered. */
		template <typename ComponentTy>
		inline [[nodiscard]] bool HasComponent(Entity entity) const noexcept;
		 
		/* Returns a raw pointer to a component type from a tied entity.
		 * May return nullptr if the entity is invalid, or the component doesn't exist.
		 * Note that if the addition of a component, or destruction of a stale component causes a resize of the component vector, any pointers
		 * retrieved before the resize will become dangling due to the semantics of std::vector. To combat this, copy the component data when
		 * ownership is needed, or refrain from persisting ComponentTy pointers across instances where components are added.
		 * A pointer is returned over a reference due to the possibility of misuse where a valid reference to a component may not exist. */
		template <typename ComponentTy>
		inline [[nodiscard]] ComponentTy* GetComponent(Entity e) noexcept;
	private:
		/* Iterates through all entities in m_ReservedEntities and compares @entity's ID to each reserved entity's ID.
		 * Returns true if a matching entity ID was found, false otherwise.
		 * TODO : Replace linear searching with a bitset or hash set for faster searching. */
		[[nodiscard]] bool IsEntityCreated(Entity entity) const noexcept;

		/* Destroys all components owned by the entity. */
		//void DestroyOwnedComponents(Entity e) noexcept;
	private:
		static constexpr size_t S_DEFAULT_POOL_SIZE = 20;
		std::vector<Entity> m_ReservedEntities;
		std::vector<Entity> m_DestroyedEntities;
		std::unordered_map<TypeID, std::unique_ptr<ISparseSet>> m_MappedComponentSets;
	};

	template <typename ComponentTy, typename... Args>
	inline bool ECS::EmplaceComponent(Entity entity, Args&&... args) noexcept
	{
		if (!IsEntityCreated(entity))
			return false;

		using SparseSetTy = SparseSet<ComponentTy, Entity>;

		TypeID componentTypeID = TypeWrangler::GetTypeID<ComponentTy>();

		auto it = m_MappedComponentSets.find(componentTypeID);

		if (it == m_MappedComponentSets.end())
			m_MappedComponentSets[componentTypeID] = std::make_unique<SparseSetTy>();

		std::unique_ptr<ISparseSet>& pSparseSetBase = m_MappedComponentSets[componentTypeID];

		// Sanity check...
		if (componentTypeID != pSparseSetBase->ID())
			return false;

		CM_ENGINE_IF_DEBUG(
			SparseSetTy * pCasted = dynamic_cast<SparseSetTy*>(pSparseSetBase.get());

			CM_ENGINE_ASSERT(pCasted != nullptr)
		);

		SparseSetTy* pSparseSet = static_cast<SparseSetTy*>(pSparseSetBase.get());

		if (pSparseSet->Contains(entity))
			return false;

		pSparseSet->EmplaceComponent(entity, std::forward<Args>(args)...);
		return true;
	}

	template <typename ComponentTy>
	inline [[nodiscard]] bool ECS::HasComponent(Entity entity) const noexcept
	{
		if (!IsEntityCreated(entity))
			return false;

		using SparseSetTy = SparseSet<ComponentTy, Entity>;

		TypeID componentTypeID = TypeWrangler::GetTypeID<ComponentTy>();

		auto it = m_MappedComponentSets.find(componentTypeID);

		if (it == m_MappedComponentSets.end())
			return false;

		std::unique_ptr<ISparseSet>& pSparseSetBase = it->second;

		CM_ASSERT(dynamic_cast<SparseSetTy*>(pSparseSetBase.get()) != nullptr);
		const SparseSetTy* pSparseSet = static_cast<const SparseSetTy*>(pSparseSetBase.get());

		return pSparseSet->Contains(entity);
	}

	template <typename ComponentTy>
	inline [[nodiscard]] ComponentTy* ECS::GetComponent(Entity entity) noexcept
	{
		if (!IsEntityCreated(entity))
			return nullptr;

		using SparseSetTy = SparseSet<ComponentTy, Entity>;

		TypeID componentTypeID = TypeWrangler::GetTypeID<ComponentTy>();

		auto it = m_MappedComponentSets.find(componentTypeID);

		if (it == m_MappedComponentSets.end())
			return nullptr;

		return static_cast<SparseSetTy*>(it->second.get())->GetComponent(entity);
	}
}