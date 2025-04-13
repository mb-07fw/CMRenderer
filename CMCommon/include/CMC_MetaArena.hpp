#pragma once

#include <vector>
#include <type_traits>
#include <cstddef>

namespace CMCommon
{
	template <typename MetaTy>
	struct CMAllocData {
		CMAllocData(size_t allocByteSize, size_t allocByteOffset, MetaTy extraMetaData) noexcept;
		~CMAllocData() = default;

		size_t AllocByteSize = 0;
		size_t AllocByteOffset = 0;
		MetaTy ExtraData;
	};

	template <typename MetaTy>
	CMAllocData<MetaTy>::CMAllocData(size_t allocByteSize, size_t allocByteOffset, MetaTy extraMetaData) noexcept
		: AllocByteSize(allocByteSize), AllocByteOffset(allocByteOffset), ExtraData(extraMetaData)
	{
	}

	template <typename MetaTy>
	class CMMetaArena
	{
	public:
		// Allocates arena with number of bytes, and reserves space for the number of metadata containers to hold.
		// (Reserves space in vector for 20 meta data containers by default)
		CMMetaArena(size_t bytes, size_t numMetaData = 20) noexcept;
		~CMMetaArena() noexcept;
	public:
		template <typename AllocTy, typename... Args>
		AllocTy* ConstructNext(MetaTy metaData, Args&&... args) noexcept;

		inline [[nodiscard]] size_t TotalAllocations() const noexcept { return m_AllocData.size(); }
	private:
		std::byte* mP_Data = nullptr;
		size_t m_CurrentOffset = 0;
		size_t m_TotalBytes = 0;
		std::vector<CMAllocData<MetaTy>> m_AllocData;
	};

	template <typename MetaTy>
	CMMetaArena<MetaTy>::CMMetaArena(size_t bytes, size_t numMetaData) noexcept
		: m_TotalBytes(bytes)
	{
		if (bytes != 0)
			mP_Data = static_cast<std::byte*>(operator new(bytes));

		m_AllocData.reserve(numMetaData);
	}

	template <typename MetaTy>
	CMMetaArena<MetaTy>::~CMMetaArena() noexcept
	{
		operator delete(mP_Data);
	}

	template <typename MetaTy>
	template <typename AllocTy, typename... Args>
	AllocTy* CMMetaArena<MetaTy>::ConstructNext(MetaTy metaData, Args&&... args) noexcept
	{
		constexpr size_t byteSize = sizeof(AllocTy);

		if (m_CurrentOffset + byteSize > m_TotalBytes)
			return nullptr;

		m_AllocData.emplace_back(byteSize, m_CurrentOffset, metaData);

		AllocTy* pData = new(mP_Data + m_CurrentOffset) AllocTy(std::forward<Args>(args)...);

		m_CurrentOffset += byteSize;

		return pData;
	}
}