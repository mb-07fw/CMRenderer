#pragma once

#include "ECS/Entity.hpp"
#include "ECS/SparseSet.hpp"
#include "ECS/Archetype.hpp"
#include "Macros.hpp"
#include "Types.hpp"

#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace std
{
	template <>
	struct hash<CMEngine::ECS::TypeID>
	{
		inline [[nodiscard]] size_t operator()(CMEngine::ECS::TypeID typeID) const noexcept
		{
			return hash<size_t>{}(typeID.ID);
		}
	};
}

namespace CMEngine::ECS
{
	template <typename Ty>
	using ECSSparseSet = SparseSet<Ty, Entity>;

	struct EntityLocation
	{
		ArchetypeID ID;
		size_t Index = 0;
	};

	class ECS
	{
	public:
		ECS() noexcept;
		~ECS() = default;
	public:
		/* Creates and stores a reserved Entity. */
		[[nodiscard]] Entity CreateEntity() noexcept;

		/* Invalidates and recycles an entity as well as it's components if any were present.
		 * Returns true if the entity was destroyed, false otherwise. */
		bool DestroyEntity(Entity entity) noexcept;

		/* Emplaces a component into a sparse set that corresponds to Ty's TypeID, provided by TypeWrangler.
		 * A component is not created if the entity is invalid.
		 * Returns true if the component was successfully emplaced, false otherwise.
		 * Returns false if the entity is invalid, either due to being destroyed or not being registered. */
		template <typename Ty, typename... Args>
		inline bool EmplaceComponent(Entity entity, Args&&... args) noexcept;

		/* Queries the Ty's corresponding ISparseSet if it contains the entity ID.
		 * Returns true if the component was registered to the component, false otherwise.
		 * Returns false if the entity is invalid, either due to being destroyed or not being registered. */
		template <typename Ty>
		inline [[nodiscard]] bool HasComponent(Entity entity) const noexcept;
		 
		/* Returns a raw pointer to a Ty from a tied entity.
		 * May return nullptr if the entity is invalid, or the component doesn't exist.
		 * Note that if the addition of a component, or destruction of a stale component causes a resize of the component vector, any pointers
		 * retrieved before the resize will become dangling due to the semantics of std::vector. To combat this, copy the component data when
		 * ownership is needed, or refrain from persisting Ty pointers across instances where components are added.
		 * A pointer is returned over a reference due to the possibility of misuse where a valid reference to a component may not exist. */
		template <typename Ty>
		inline [[nodiscard]] View<Ty> TryGetComponent(Entity e) noexcept;

		/* Returns a reference to a Ty from a tied entity.
		 * Due to a reference being returned, this is ONLY safe if it is guaranteed that @e is tied to an instance of Ty.
		 * Same lifetime semantics apply from TryGetComponent. */
		template <typename Ty>
		inline [[nodiscard]] Ty& GetComponent(Entity e) noexcept;

		/* Returns a raw pointer to a ECSSparseSet of a provided component type.
		 * May return nullptr if the ECSSparseSet<Ty> hasn't been created yet. (No components of Ty have been stored yet)
		 * Pointers to sparse sets are guaranteed to be valid unless the underlying ECSSparseSet instance is explicity destroyed.
		 * A pointer is returned over a reference due to the possibility of misuse where a valid reference to a ECSSparseSet may not exist. */
		template <typename Ty>
		inline [[nodiscard]] View<ECSSparseSet<Ty>> GetSparseSet() noexcept;

		/* Returns a View to an Archetype that contains storage for each provided type.
		 * May return a null View if the specific Archetype<Types...> instantiation has already been created. */
		template <typename... Types>
		inline View<Archetype<Types...>> CreateArchetype() noexcept;

		template <typename... Types>
		inline [[nodiscard]] bool DestroyArchetype() noexcept;

		template <typename... Types>
		inline [[nodiscard]] bool DestroyArchetype(View<Archetype<Types...>>& archetype) noexcept;

		template <typename... Types>
		inline [[nodiscard]] View<Archetype<Types...>> GetArchetype() noexcept;

		template <typename AddTy, typename... Types, typename... LastArgs>
		inline [[nodiscard]] View<ArchetypeAdded<AddTy, Types...>> ArchetypeAdd(
			Entity e,
			Archetype<Types...>& archetype,
			LastArgs&&... lastArgs
		)  noexcept;

		template <ArchetypeType Arch, typename... ParamsTypes>
		inline bool EmplaceRow(Entity e, Arch& archetype, ParamsTypes&&... paramsObjs) noexcept;
	private:
		/* Iterates through all entities in m_ReservedEntities and compares @entity's ID to each reserved entity's ID.
		 * Returns true if a matching entity ID was found, false otherwise.
		 * TODO : Replace linear searching with a bitset or hash set for faster searching. */
		[[nodiscard]] bool IsEntityCreated(Entity entity) const noexcept;

		[[nodiscard]] bool IsMappedToArchetype(Entity e) const noexcept;
		void UnmapFromArchetype(Entity e) noexcept;

		/* Destroys all components owned by the entity. */
		//void DestroyOwnedComponents(Entity e) noexcept;
	private:
		static constexpr size_t S_Default_Entity_Pool_Size = 50;
		std::vector<Entity> m_ReservedEntities;
		std::vector<Entity> m_DestroyedEntities;

		std::unordered_map<TypeID, std::unique_ptr<ISparseSet>> m_SparseSets;

		/* Used to map Entities to their corresponding locations in their mapped archetypes. */
		std::vector<size_t> m_SparseIDs;
		std::vector<EntityLocation> m_DenseLocations;
		std::unordered_map<ArchetypeID, std::unique_ptr<IArchetype>> m_Archetypes;
	};

	template <typename Ty, typename... Args>
	inline bool ECS::EmplaceComponent(Entity entity, Args&&... args) noexcept
	{
		using SparseSetTy = ECSSparseSet<Ty>;

		if (!IsEntityCreated(entity))
			return false;

		TypeID typeID = GetTypeID<Ty>();

		if (m_SparseSets.find(typeID) == m_SparseSets.end())
			m_SparseSets[typeID] = std::make_unique<SparseSetTy>();

		View<SparseSetTy> sparseSet = GetSparseSet<Ty>();

		if (sparseSet.Null() || sparseSet->Contains(entity))
			return false;

		// Sanity check...
		CM_ENGINE_IF_DEBUG(
			if (sparseSet->ID() != typeID)
				return false;
		)

		sparseSet->EmplaceComponent(entity, std::forward<Args>(args)...);
		return true;
	}

	template <typename Ty>
	inline [[nodiscard]] bool ECS::HasComponent(Entity entity) const noexcept
	{
		using SparseSetTy = ECSSparseSet<Ty>;

		if (!IsEntityCreated(entity))
			return false;

		View<SparseSetTy> sparseSet = GetSparseSet<Ty>();

		return sparseSet.Null() || 
			sparseSet->Contains(entity);
	}

	template <typename Ty>
	inline [[nodiscard]] View<Ty> ECS::TryGetComponent(Entity entity) noexcept
	{
		using SparseSetTy = ECSSparseSet<Ty>;
		using ReturnTy = View<Ty>;

		TypeID typeID = GetTypeID<Ty>();
		View<SparseSetTy> sparseSet = GetSparseSet<Ty>();
	
		if (sparseSet.Null())
			return ReturnTy::NullView();

		return ReturnTy(sparseSet->Get(entity));
	}

	template <typename Ty>
	inline [[nodiscard]] Ty& ECS::GetComponent(Entity entity) noexcept
	{
		View<Ty> comp = TryGetComponent<Ty>(entity);
		CM_ENGINE_ASSERT(comp.NonNull());
		return comp.Ref();
	}

	template <typename Ty>
	inline [[nodiscard]] View<ECSSparseSet<Ty>> ECS::GetSparseSet() noexcept
	{
		using SparseSetTy = ECSSparseSet<Ty>;
		using SparseSetPtr = SparseSetTy*;
		using ViewTy = View<SparseSetTy>;

		auto it = m_SparseSets.find(GetTypeID<Ty>());
		if (it == m_SparseSets.end())
			return ViewTy::NullView();

		const std::unique_ptr<ISparseSet>& pBase = it->second;

		CM_ENGINE_ASSERT(TryCast<SparseSetTy*>(pBase.get()) != nullptr);
		SparseSetPtr pDerived = Cast<SparseSetPtr>(pBase.get());

		return ViewTy(pDerived);
	}

	/* Returns a View to an Archetype that contains storage for each provided type.
     * May return a null View if the specific Archetype<Types...> instantiation has already been created. */
	template <typename... Types>
	inline [[nodiscard]] View<Archetype<Types...>> ECS::CreateArchetype() noexcept
	{
		using ArchetypeTy = Archetype<Types...>;
		using ViewTy = View<ArchetypeTy>;

		ArchetypeID id = GetArchetypeID<Types...>();

		/* An archetype with the same type configuration already exists. */
		if (m_Archetypes.find(id) != m_Archetypes.end())
			return ViewTy::NullView();

		std::unique_ptr<ArchetypeTy> pArchetype = std::make_unique<ArchetypeTy>();

		ViewTy archetype(pArchetype.get());

		m_Archetypes[id] = std::move(pArchetype);
		return archetype;
	}

	template <typename... Types>
	inline [[nodiscard]] bool ECS::DestroyArchetype() noexcept
	{
		auto it = m_Archetypes.find(GetArchetypeID<Types...>());
		if (it == m_Archetypes.end())
			return false;

		m_Archetypes.erase(it);
		return true;
	}

	template <typename... Types>
	inline [[nodiscard]] bool ECS::DestroyArchetype(View<Archetype<Types...>>& archetype) noexcept
	{
		if (archetype.Null())
			return false;

		auto it = m_Archetypes.find(archetype->ID());
		if (it == m_Archetypes.end())
			return false;

		m_Archetypes.erase(it);
		archetype.Reset();

		return true;
	}

	template <typename... Types>
	inline [[nodiscard]] View<Archetype<Types...>> ECS::GetArchetype() noexcept
	{
		using ArchetypeTy = Archetype<Types...>;
		using ArchetypePtr = ArchetypeTy*;
		using ViewTy = View<ArchetypeTy>;

		auto it = m_Archetypes.find(GetArchetypeID<Types...>());
		if (it == m_Archetypes.end())
			return ViewTy::NullView();

		IArchetype* pBase = it->second.get();
		CM_ENGINE_ASSERT(TryCast<ArchetypePtr>(pBase) != nullptr);

		return ViewTy(Cast<ArchetypePtr>(pBase));
	}

	template <typename AddTy, typename... Types, typename... LastArgs>
	inline [[nodiscard]] View<ArchetypeAdded<AddTy, Types...>> ECS::ArchetypeAdd(
		Entity e,
		Archetype<Types...>& archetype,
		LastArgs&&... lastArgs
	) noexcept
	{
		using NewArchetypeTy = ArchetypeAdded<AddTy, Types...>;
		using ViewTy = View<NewArchetypeTy>;

		if (!IsMappedToArchetype(e))
			return ViewTy::NullView();

		std::unique_ptr<NewArchetypeTy> pNewArchetype = std::make_unique<NewArchetypeTy>();
		ViewTy newArchetype = pNewArchetype.get();

		EntityLocation& location = m_DenseLocations[m_SparseIDs[e.Index()]];
		size_t previousIndex = location.Index;

		location.ID = newArchetype->ID();
		location.Index = newArchetype->Size();

		newArchetype->TransferBack(
			archetype,
			previousIndex,
			std::forward<LastArgs>(lastArgs)...
		);
		
		m_Archetypes[location.ID] = std::move(pNewArchetype);
		return newArchetype;
	}

	template <ArchetypeType Arch, typename... ParamsTypes>
	inline bool ECS::EmplaceRow(Entity e, Arch& archetype, ParamsTypes&&... paramsObjs) noexcept
	{
		/* Another archetype already "owns" the entity. */
		if (IsMappedToArchetype(e))
			return false;

		if (e.Index() >= m_SparseIDs.size())
			m_SparseIDs.resize(((size_t)(e.Index()) + 1) * 2);

		EntityLocation location = {};
		location.ID = archetype.ID();
		location.Index = archetype.Size();

		m_SparseIDs[e.Index()] = m_DenseLocations.size();
		m_DenseLocations.emplace_back(location);

		archetype.EmplaceBack(std::forward<ParamsTypes>(paramsObjs)...);
		return true;
	}
}