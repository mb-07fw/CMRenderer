#pragma once

#include "TypeID.hpp"

#include <cstdint>
#include <vector>
#include <utility>
#include <type_traits>

namespace CMEngine::ECS
{
	class ISparseSet
	{
	public:
		inline ISparseSet(TypeID typeID) noexcept;

		virtual ~ISparseSet() = default;
	public:
		inline [[nodiscard]] TypeID ID() const noexcept { return m_TypeID; }
	protected:
		TypeID m_TypeID = {};
	};

	inline ISparseSet::ISparseSet(TypeID typeID) noexcept
		: m_TypeID(typeID)
	{
	}

	template <typename IDTy>
	concept CustomID = requires(const IDTy & id) {
		{ id.ToIndex() } -> std::convertible_to<size_t>;
	};

	template <typename IDTy>
	concept ValidIDType = CustomID<IDTy> || std::unsigned_integral<IDTy>;

	template <typename ComponentTy, typename IDTy = size_t>
		requires ValidIDType<IDTy>
	class SparseSet : public ISparseSet
	{
	public:
		inline SparseSet() noexcept;
		~SparseSet() = default;
	public:
		inline [[nodiscard]] bool Contains(IDTy id) const noexcept;
		inline void Remove(IDTy id) noexcept;

		template <typename... Args>
		inline void EmplaceComponent(IDTy id, Args&&... args) noexcept;

		inline [[nodiscard]] ComponentTy* GetComponent(IDTy id) noexcept;
		inline [[nodiscard]] const ComponentTy* GetComponent(IDTy id) const noexcept;

		/* m_SparseArray: Maps ID → it's Index in m_DenseArray.
		 * m_DenseArray : Stores ID's contiguously.
		 * m_Components : Stores Components contiguously, parallel to m_DenseArray.
		 */
		inline [[nodiscard]] const std::vector<size_t>& Sparse() const noexcept { return m_SparseArray; }
		inline [[nodiscard]] const std::vector<IDTy>& Dense() const noexcept { return m_DenseArray; }
		inline [[nodiscard]] const std::vector<ComponentTy>& Components() const noexcept { return m_Components; }
	private:
		inline void Insert(IDTy id) noexcept;

		inline constexpr [[nodiscard]] size_t AsIndex(IDTy id) const noexcept;
	private:
		std::vector<size_t> m_SparseArray;
		std::vector<IDTy> m_DenseArray;
		std::vector<ComponentTy> m_Components;
#undef max
		constexpr static size_t S_INVALID_INDEX = std::numeric_limits<size_t>::max();
	};

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	inline SparseSet<ComponentTy, IDTy>::SparseSet() noexcept
		: ISparseSet(TypeWrangler::GetTypeID<ComponentTy>())
	{
	}

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	inline [[nodiscard]] bool SparseSet<ComponentTy, IDTy>::Contains(IDTy id) const noexcept
	{
		size_t sparseIndex = AsIndex(id);

		if (sparseIndex >= m_SparseArray.size() || sparseIndex == S_INVALID_INDEX)
			return false;

		size_t denseIndex = m_SparseArray[sparseIndex];

		/* BUG: Capacity is 0 when it should match the number of components currently stored. */
		return denseIndex < m_Components.size() &&	  // denseIndex at sparseIndex is in bounds of m_Components.
			AsIndex(m_DenseArray[denseIndex]) == sparseIndex;  // Entity id's sparse index stored in m_DenseArray matches the sparseIndex.
	}

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	inline void SparseSet<ComponentTy, IDTy>::Remove(IDTy id) noexcept
	{
		if (!Contains(id))
			return;

		size_t sparseIndex = AsIndex(id);
		size_t denseIndex = m_SparseArray[sparseIndex];

		if (m_Components.size() > 1)
		{
			m_Components[denseIndex] = m_Components.back();
			m_DenseArray[denseIndex] = m_DenseArray.back();
		}

		m_Components.pop_back();
		m_DenseArray.pop_back();

		m_SparseArray[sparseIndex] = S_INVALID_INDEX;
	}

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	template <typename... Args>
	inline void SparseSet<ComponentTy, IDTy>::EmplaceComponent(IDTy id, Args&&... args) noexcept
	{
		if (Contains(id))
			return;

		Insert(id);

		if (m_Components.size() >= m_Components.capacity())
			m_Components.reserve((m_Components.size() + 1) * 2);

		m_Components.emplace_back(std::forward<Args>(args)...);
	}

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	inline [[nodiscard]] ComponentTy* SparseSet<ComponentTy, IDTy>::GetComponent(IDTy id) noexcept
	{
		if (!Contains(id))
			return nullptr;

		return &m_Components[m_SparseArray[AsIndex(id)]];
	}

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	inline [[nodiscard]] const ComponentTy* SparseSet<ComponentTy, IDTy>::GetComponent(IDTy id) const noexcept
	{
		if (!Contains(id))
			return nullptr;

		return &m_Components[m_SparseArray[AsIndex(id)]];
	}

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	inline void SparseSet<ComponentTy, IDTy>::Insert(IDTy id) noexcept
	{
		size_t sparseIndex = AsIndex(id);

		if (sparseIndex >= m_SparseArray.size())
		{
			size_t newSize = (sparseIndex + 1) * 2;
			m_SparseArray.resize(newSize);
		}

		m_SparseArray[sparseIndex] = m_DenseArray.size();
		m_DenseArray.emplace_back(id);
	}

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	inline constexpr [[nodiscard]] size_t SparseSet<ComponentTy, IDTy>::AsIndex(IDTy id) const noexcept
	{
		if constexpr (CustomID<IDTy>)
			return static_cast<size_t>(id.ToIndex());
		else
			return static_cast<size_t>(id);
	}
}