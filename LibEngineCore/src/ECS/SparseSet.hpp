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
		{ id.Index() } -> std::convertible_to<size_t>;
	};

	template <typename IDTy>
	concept ValidIDType = CustomID<IDTy> || std::unsigned_integral<IDTy>;

	template <typename Ty, typename IDTy = size_t>
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

		inline [[nodiscard]] Ty* Get(IDTy id) noexcept;
		inline [[nodiscard]] const Ty* Get(IDTy id) const noexcept;

		inline [[nodiscard]] std::vector<size_t>& Sparse() noexcept { return m_SparseArray; }
		inline [[nodiscard]] const std::vector<size_t>& Sparse() const noexcept { return m_SparseArray; }

		inline [[nodiscard]] std::vector<IDTy>& Dense() noexcept { return m_DenseArray; }
		inline [[nodiscard]] const std::vector<IDTy>& Dense() const noexcept { return m_DenseArray; }

		inline [[nodiscard]] std::vector<Ty>& Data() noexcept { return m_Data; }
		inline [[nodiscard]] const std::vector<Ty>& Data() const noexcept { return m_Data; }
	private:
		inline void Insert(IDTy id) noexcept;

		inline constexpr [[nodiscard]] size_t AsIndex(IDTy id) const noexcept;
	private:
		/* m_SparseArray: Maps ID → it's Index in m_DenseArray.
		 * m_DenseArray : Stores ID's contiguously. (used to map indexes into m_Data back to entity ID's)
		 * m_Data : Stores Ty's contiguously, parallel to m_DenseArray.
		 */
		std::vector<size_t> m_SparseArray;
		std::vector<IDTy> m_DenseArray;
		std::vector<Ty> m_Data;
#undef max
		constexpr static size_t S_Removed_Index = std::numeric_limits<size_t>::max();
	};

	template <typename Ty, typename IDTy>
		requires ValidIDType<IDTy>
	inline SparseSet<Ty, IDTy>::SparseSet() noexcept
		: ISparseSet(GetTypeID<Ty>())
	{
	}

	template <typename Ty, typename IDTy>
		requires ValidIDType<IDTy>
	inline [[nodiscard]] bool SparseSet<Ty, IDTy>::Contains(IDTy id) const noexcept
	{
		size_t sparseIndex = AsIndex(id);

		if (sparseIndex >= m_SparseArray.size() || 
			sparseIndex == S_Removed_Index)
			return false;

		size_t denseIndex = m_SparseArray[sparseIndex];

		/* BUG: Capacity is 0 when it should match the number of components currently stored. */
		return denseIndex < m_Data.size() &&	  // denseIndex at sparseIndex is in bounds of m_Data.
			AsIndex(m_DenseArray[denseIndex]) == sparseIndex;  // Entity id's sparse index stored in m_DenseArray matches the sparseIndex.
	}

	template <typename Ty, typename IDTy>
		requires ValidIDType<IDTy>
	inline void SparseSet<Ty, IDTy>::Remove(IDTy id) noexcept
	{
		if (!Contains(id))
			return;

		size_t sparseIndex = AsIndex(id);
		size_t denseIndex = m_SparseArray[sparseIndex];

		if (m_Data.size() > 1)
		{
			m_Data[denseIndex] = m_Data.back();
			m_DenseArray[denseIndex] = m_DenseArray.back();
		}

		m_Data.pop_back();
		m_DenseArray.pop_back();

		m_SparseArray[sparseIndex] = S_Removed_Index;
	}

	template <typename Ty, typename IDTy>
		requires ValidIDType<IDTy>
	template <typename... Args>
	inline void SparseSet<Ty, IDTy>::EmplaceComponent(IDTy id, Args&&... args) noexcept
	{
		if (Contains(id))
			return;

		Insert(id);

		if (m_Data.size() >= m_Data.capacity())
			m_Data.reserve((m_Data.size() + 1) * 2);

		m_Data.emplace_back(std::forward<Args>(args)...);
	}

	template <typename Ty, typename IDTy>
		requires ValidIDType<IDTy>
	inline [[nodiscard]] Ty* SparseSet<Ty, IDTy>::Get(IDTy id) noexcept
	{
		if (!Contains(id))
			return nullptr;

		return &m_Data[m_SparseArray[AsIndex(id)]];
	}

	template <typename Ty, typename IDTy>
		requires ValidIDType<IDTy>
	inline [[nodiscard]] const Ty* SparseSet<Ty, IDTy>::Get(IDTy id) const noexcept
	{
		if (!Contains(id))
			return nullptr;

		return &m_Data[m_SparseArray[AsIndex(id)]];
	}

	template <typename Ty, typename IDTy>
		requires ValidIDType<IDTy>
	inline void SparseSet<Ty, IDTy>::Insert(IDTy id) noexcept
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

	template <typename Ty, typename IDTy>
		requires ValidIDType<IDTy>
	inline constexpr [[nodiscard]] size_t SparseSet<Ty, IDTy>::AsIndex(IDTy id) const noexcept
	{
		if constexpr (CustomID<IDTy>)
			return static_cast<size_t>(id.Index());
		else
			return static_cast<size_t>(id);
	}
}