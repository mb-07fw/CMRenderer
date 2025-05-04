#pragma once

#include <cstdint>
#include <vector>
#include <functional>

#include <chrono>

#include "CMC_ECSTypeID.hpp"
#include "CMC_ECSEntity.hpp"

namespace CMCommon
{
	class ICMSparseSet
	{
	public:
		inline ICMSparseSet(
			CMECSTypeID typeID,
			std::function<bool(CMECSEntity)> containsFunc,
			std::function<void(CMECSEntity)> removeFunc
		) noexcept;

		virtual ~ICMSparseSet() = default;
	public:
		inline [[nodiscard]] bool ContainsEntity(CMECSEntity entity) const noexcept;
		inline void RemoveEntity(CMECSEntity entity) noexcept;

		inline [[nodiscard]] CMECSTypeID TypeID() const noexcept { return m_TypeID; }
	protected:
		CMECSTypeID m_TypeID = {};
		std::function<bool(CMECSEntity)> m_ContainsFunc;
		std::function<void(CMECSEntity)> m_RemoveFunc;
	};

	inline ICMSparseSet::ICMSparseSet(
		CMECSTypeID typeID,
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

	template <typename ComponentTy>
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
		std::vector<size_t> m_SparseArray;
		std::vector<size_t> m_DenseArray;
		std::vector<ComponentTy> m_Components;

		/* m_SparseArray: Maps EntityID → Index in m_DenseArray.
		 *
		 * m_DenseArray : Stores EntityIDs contiguously.
		 *
		 * m_Components : Stores Components contiguously, parallel to m_DenseArray.
		 */
	};

	template <typename ComponentTy>
	inline CMSparseSet<ComponentTy>::CMSparseSet() noexcept
		: ICMSparseSet(
			CMECSTypeWrangler::GetTypeID<ComponentTy>(),
			std::bind(&CMSparseSet<ComponentTy>::ContainsEntity, this, std::placeholders::_1),
			std::bind(&CMSparseSet<ComponentTy>::RemoveEntity, this, std::placeholders::_1)
		  )
	{
	}

	template <typename ComponentTy>
	inline [[nodiscard]] bool CMSparseSet<ComponentTy>::ContainsEntity(CMECSEntity entity) const noexcept
	{
		uint32_t entityIndex = entity.ToIndex();

		if (entityIndex >= m_SparseArray.size())
			return false;

		size_t denseIndex = m_SparseArray[entityIndex];

		return denseIndex < m_Components.size() &&		// Dense index at id is in bounds of m_Components.
			m_DenseArray[denseIndex] == entityIndex;	// Entity id stored in m_DenseArray matches the entity id.
	}

	template <typename ComponentTy>
	inline void CMSparseSet<ComponentTy>::RemoveEntity(CMECSEntity entity) noexcept
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

	template <typename ComponentTy>
	template <typename... Args>
	inline void CMSparseSet<ComponentTy>::EmplaceComponent(CMECSEntity entity, Args&&... args) noexcept
	{
		if (ContainsEntity(entity))
			return;

		InsertEntity(entity);

		if (m_Components.size() >= m_Components.capacity())
			m_Components.reserve((m_Components.size() + 1) * 2);
		
		m_Components.emplace_back(std::forward<Args>(args)...);
	}

	template <typename ComponentTy>
	inline [[nodiscard]] ComponentTy* CMSparseSet<ComponentTy>::GetComponent(CMECSEntity entity) noexcept
	{
		if (!ContainsEntity(entity))
			return nullptr;

		return &m_Components[m_SparseArray[entity.ToIndex()]];
	}

	template <typename ComponentTy>
	inline void CMSparseSet<ComponentTy>::InsertEntity(CMECSEntity entity) noexcept
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