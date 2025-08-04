#pragma once

#include <cstdint>
#include <vector>
#include <type_traits>

#include "Common/TypeID.hpp"

namespace CMEngine::Core
{
	class ISparseSet
	{
	public:
		inline ISparseSet(Common::TypeID typeID) noexcept;

		virtual ~ISparseSet() = default;
	public:
		inline [[nodiscard]] Common::TypeID TypeID() const noexcept { return m_TypeID; }
	protected:
		Common::TypeID m_TypeID = {};
	};

	inline ISparseSet::ISparseSet(Common::TypeID typeID) noexcept
		: m_TypeID(typeID)
	{
	}

	template <typename IDTy>
	concept CustomID = requires(const IDTy & id) {
		{ id.ToIndex() } -> std::convertible_to<size_t>;
	};

	template <typename IDTy>
	concept UIntegralID = std::unsigned_integral<IDTy>;

	template <typename IDTy>
	concept ValidIDType = CustomID<IDTy> || UIntegralID<IDTy>;

	template <typename ComponentTy, typename IDTy = size_t>
		requires ValidIDType<IDTy>
	class SparseSet : public ISparseSet
	{
	private:
		consteval static auto GetIndexType() noexcept;
	public:
		using IndexTy = decltype(GetIndexType());


	public:
		inline SparseSet() noexcept;
		~SparseSet() = default;
	public:
		inline [[nodiscard]] bool Contains(IDTy id) const noexcept; // Provided to ISparseSet via std::function<bool()>
		inline void Remove(IDTy id) noexcept;						 // Provided to ISparseSet via std::function<void()>

		template <typename... Args>
		inline void EmplaceComponent(IDTy id, Args&&... args) noexcept;

		inline [[nodiscard]] ComponentTy* GetComponent(IDTy id) noexcept;
	private:
		inline void Insert(IDTy id) noexcept;

		inline [[nodiscard]] size_t AsIndex(IDTy id) const noexcept;
	private:
		std::vector<IndexTy> m_SparseArray;
		std::vector<IndexTy> m_DenseArray;
		std::vector<ComponentTy> m_Components;

		/* m_SparseArray: Maps ID → it's Index in m_DenseArray.
		 * m_DenseArray : Stores ID's contiguously.
		 * m_Components : Stores Components contiguously, parallel to m_DenseArray.
		 */

		constexpr static IndexTy S_INVALID_INDEX = std::numeric_limits<IndexTy>::max();
	};

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	consteval auto SparseSet<ComponentTy, IDTy>::GetIndexType() noexcept
	{
		if constexpr (CustomID<IDTy>)
			return std::declval<decltype(std::declval<IDTy>().ToIndex())>();
		else
			return std::declval<IDTy>();
	}

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	inline SparseSet<ComponentTy, IDTy>::SparseSet() noexcept
		: ISparseSet(Common::TypeWrangler::GetTypeID<ComponentTy>())
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
		return denseIndex < m_Components.size() &&	  // Dense index at id is in bounds of m_Components.
			m_DenseArray[denseIndex] == sparseIndex;  // Entity id stored in m_DenseArray matches the entity's id
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
	inline void SparseSet<ComponentTy, IDTy>::Insert(IDTy id) noexcept
	{
		size_t sparseIndex = AsIndex(id);

		if (sparseIndex >= m_SparseArray.size())
		{
			size_t newSize = (sparseIndex + 1) * 2;
			m_SparseArray.resize(newSize);
		}

		m_SparseArray[sparseIndex] = static_cast<IndexTy>(m_DenseArray.size());
		m_DenseArray.emplace_back(static_cast<IndexTy>(sparseIndex));
	}

	template <typename ComponentTy, typename IDTy>
		requires ValidIDType<IDTy>
	inline [[nodiscard]] size_t SparseSet<ComponentTy, IDTy>::AsIndex(IDTy id) const noexcept
	{
		if constexpr (CustomID<IDTy>)
			return static_cast<size_t>(id.ToIndex());
		else
			return static_cast<size_t>(id);
	}
}