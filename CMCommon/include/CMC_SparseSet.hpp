#pragma once

#include <cstdint>
#include <vector>
#include <functional>

#include <chrono>

#include "CMC_ECSEntity.hpp"
#include "CMC_TypeID.hpp"

namespace CMCommon
{
	class ICMSparseSet
	{
	public:
		inline ICMSparseSet(
			CMTypeID typeID,
			std::function<bool(CMECSEntity)> containsFunc,
			std::function<void(CMECSEntity)> removeFunc
		) noexcept;

		virtual ~ICMSparseSet() = default;
	public:
		inline [[nodiscard]] bool ContainsEntity(CMECSEntity entity) const noexcept;
		inline void RemoveEntity(CMECSEntity entity) noexcept;

		inline [[nodiscard]] CMTypeID TypeID() const noexcept { return m_TypeID; }
	protected:
		CMTypeID m_TypeID = {};
		std::function<bool(CMECSEntity)> m_ContainsFunc;
		std::function<void(CMECSEntity)> m_RemoveFunc;
	};

	inline ICMSparseSet::ICMSparseSet(
		CMTypeID typeID,
		std::function<bool(CMECSEntity)> containsFunc,
		std::function<void(CMECSEntity)> removeFunc
	) noexcept
		: m_TypeID(typeID),
		  m_ContainsFunc(containsFunc),
		  m_RemoveFunc(removeFunc)
	{
	}

	inline [[nodiscard]] bool ICMSparseSet::ContainsEntity(CMECSEntity entity) const noexcept
	{
		return m_ContainsFunc(entity);
	}

	inline void ICMSparseSet::RemoveEntity(CMECSEntity entity) noexcept
	{
		return m_RemoveFunc(entity);
	}

	template <typename ComponentTy, typename IDTy = size_t>
	class CMSparseSet : public ICMSparseSet
	{
	public:
		inline CMSparseSet() noexcept;
		~CMSparseSet() = default;
	public:
		inline [[nodiscard]] bool ContainsEntity(CMECSEntity entity) const noexcept; // Provided to ICMSparseSet via std::function<bool()>
		inline void RemoveEntity(CMECSEntity entity) noexcept;						 // Provided to ICMSparseSet via std::function<void()>

		template <typename... Args>
		inline void EmplaceComponent(CMECSEntity entity, Args&&... args) noexcept;

		inline [[nodiscard]] ComponentTy* GetComponent(CMECSEntity entity) noexcept;
	private:
		inline void InsertEntity(CMECSEntity entity) noexcept;
	private:
		std::vector<IDTy> m_SparseArray;
		std::vector<IDTy> m_DenseArray;
		std::vector<ComponentTy> m_Components;

		/* m_SparseArray: Maps EntityID → Index in m_DenseArray.
		 *
		 * m_DenseArray : Stores EntityIDs contiguously.
		 *
		 * m_Components : Stores Components contiguously, parallel to m_DenseArray.
		 */
	};

	template <typename ComponentTy, typename IDTy>
	inline CMSparseSet<ComponentTy, IDTy>::CMSparseSet() noexcept
		: ICMSparseSet(
			CMTypeWrangler::GetTypeID<ComponentTy>(),
			[this](CMECSEntity entity) { return ContainsEntity(entity); },
			[this](CMECSEntity entity) { RemoveEntity(entity); }
		  )
	{
	}

	template <typename ComponentTy, typename IDTy>
	inline [[nodiscard]] bool CMSparseSet<ComponentTy, IDTy>::ContainsEntity(CMECSEntity entity) const noexcept
	{
		uint32_t entityIndex = entity.ToIndex();

		if (entityIndex >= m_SparseArray.size())
			return false;

		size_t denseIndex = m_SparseArray[entityIndex];

		return denseIndex < m_Components.size() &&		// Dense index at id is in bounds of m_Components.
			m_DenseArray[denseIndex] == entityIndex;	// Entity id stored in m_DenseArray matches the entity's id
	}

	template <typename ComponentTy, typename IDTy>
	inline void CMSparseSet<ComponentTy, IDTy>::RemoveEntity(CMECSEntity entity) noexcept
	{
		if (!ContainsEntity(entity))
			return;

		uint32_t entityIndex = entity.ToIndex();

		size_t denseIndex = m_SparseArray[entityIndex];
		
		if (m_Components.size() > 1)
		{
			m_Components[denseIndex] = m_Components.back();
			m_DenseArray[denseIndex] = m_DenseArray.back();
		}

		m_Components.pop_back();
		m_DenseArray.pop_back();

		m_SparseArray[entityIndex] = 0;
	}

	template <typename ComponentTy, typename IDTy>
	template <typename... Args>
	inline void CMSparseSet<ComponentTy, IDTy>::EmplaceComponent(CMECSEntity entity, Args&&... args) noexcept
	{
		if (ContainsEntity(entity))
			return;

		InsertEntity(entity);

		if (m_Components.size() >= m_Components.capacity())
			m_Components.reserve((m_Components.size() + 1) * 2);
		
		m_Components.emplace_back(std::forward<Args>(args)...);
	}

	template <typename ComponentTy, typename IDTy>
	inline [[nodiscard]] ComponentTy* CMSparseSet<ComponentTy, IDTy>::GetComponent(CMECSEntity entity) noexcept
	{
		if (!ContainsEntity(entity))
			return nullptr;

		return &m_Components[m_SparseArray[entity.ToIndex()]];
	}

	template <typename ComponentTy, typename IDTy>
	inline void CMSparseSet<ComponentTy, IDTy>::InsertEntity(CMECSEntity entity) noexcept
	{
		uint32_t entityIndex = entity.ToIndex();

		if (entityIndex >= m_SparseArray.size())
		{
			size_t newSize = (entityIndex + 1) * 2;
			m_SparseArray.resize(newSize);
		}

		m_SparseArray[entityIndex] = m_DenseArray.size();
		m_DenseArray.emplace_back(entityIndex);
	}
}