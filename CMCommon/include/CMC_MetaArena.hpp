#pragma once

#include <vector>
#include <type_traits>
#include <cstdint>
#include <cstddef>

#include "CMC_Logger.hpp"

namespace CMCommon
{
	enum class CMAllocType : int8_t
	{
		INVALID = -127,
		FAILED_ALREADY_ALLOCATED,
		FAILED_NOT_ALLOCATED,
		FAILED_ALLOC_INSANE_SIZE,
		FAILED_BAD_ALLOC,
		FAILED_BYTE_SIZE_OVERFLOWING_ARENA,
		FAILED = 0,
		SUCCEEDED = 1
	};

	inline constexpr [[nodiscard]] int8_t AllocTypeToInt8(CMAllocType type) noexcept
	{
		return static_cast<int8_t>(type);
	}

	inline constexpr [[nodiscard]] bool AllocSucceeded(CMAllocType type) noexcept
	{
		return AllocTypeToInt8(type) > AllocTypeToInt8(CMAllocType::FAILED);
	}

	inline constexpr [[nodiscard]] bool AllocFailed(CMAllocType type) noexcept
	{
		return AllocTypeToInt8(type) < AllocTypeToInt8(CMAllocType::SUCCEEDED);
	}

	struct CMAllocMetaData
	{
		inline constexpr CMAllocMetaData(size_t bytes, size_t byteOffset) noexcept
			: Bytes(bytes),
			  ByteOffset(byteOffset)
		{
		}

		~CMAllocMetaData() = default;

		size_t Bytes = 0;
		size_t ByteOffset = 0;
	};

	template <typename Ty>
	struct CMAllocHandle
	{
		inline constexpr CMAllocHandle(size_t metaDataIndex, CMAllocType result, Ty* pData) noexcept
			: MetaDataIndex(MetaDataIndex),
			  Result(result),
			  pData(pData)
		{
		}

		~CMAllocHandle() = default;

		size_t MetaDataIndex = 0;
		CMAllocType Result = CMAllocType::INVALID; /* Note to self; CMAllocType::INVALID
													* implies an un-initialized alloc handle.
													*/
		Ty* pData = nullptr;
	};

	class CMMetaArena
	{
	public:
		CMMetaArena(CMCommon::CMLoggerWide& logger) noexcept;
		~CMMetaArena() noexcept;
	public:
		/* Allocates the initial block of memory for the arena.
		 * 
		 * Returns CMAllocType::SUCCEEDED if allocation succeeds;
		 *   any CMAllocType < 1 otherwise.
		 * 
		 * Use AllocSucceeded to check if returned CMAllocType is a 'succeeded' type.
		 */
		[[nodiscard]] CMAllocType Allocate(size_t bytes) noexcept;

		/* De-allocates the arena's block of memory.
		 * 
		 * Returns CMAllocType::SUCCEEDED if de-allocation succeeds;
		 *   any CMAllocType < 1 otherwise.
		 *
		 * Use AllocSucceeded to check if returned CMAllocType is a 'succeeded' type.
		 *
		 * NOTE: May cause UB for any objects not destroyed before de-allocation.
		 */
		CMAllocType Deallocate() noexcept;

		template <typename Ty, typename... Args>
			requires std::is_trivially_copyable_v<Ty>
		inline [[nodiscard]] CMAllocHandle<Ty> ConstructNext(Args&&... args) noexcept;

		inline [[nodiscard]] size_t TotalAllocations() const noexcept { return m_AllocData.size(); }
	private:
		static constexpr size_t S_ALLOC_BYTE_SIZE_SANITY_BOUNDARY = 10'000;
		static constexpr size_t S_INVALID_META_DATA_INDEX = static_cast<size_t>(-1);
		std::byte* mP_Data = nullptr;
		size_t m_CurrentByteOffset = 0;
		size_t m_TotalBytes = 0;
		bool m_AllocatedBlock = false;
		std::vector<CMAllocMetaData> m_AllocData;
		CMCommon::CMLoggerWide& m_Logger;
	};

	template <typename Ty, typename... Args>
		requires std::is_trivially_copyable_v<Ty>
	inline [[nodiscard]] CMAllocHandle<Ty> CMMetaArena::ConstructNext(Args&&... args) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMetaArena [ConstructNext] | ";

		if (!m_AllocatedBlock)
		{
			m_Logger.LogWarningNLTagged(FuncTag, L"Arena hasn't allocated a block of memory yet.");
			return CMAllocType::FAILED_NOT_ALLOCATED;
		}

		size_t nextOffset = m_CurrentByteOffset + sizeof(Ty);

		if (nextOffset > m_TotalBytes)
		{
			m_Logger.LogWarningNLFormatted(
				FuncTag,
				L"Allocation of size `{}` would overflow current byte offset of `{}`.",
				sizeof(Ty), m_CurrentByteOffset
			);

			return CMAllocHandle<Ty>(
				S_INVALID_META_DATA_INDEX,
				CMAllocType::FAILED_BYTE_SIZE_OVERFLOWING_ARENA,
				nullptr
			);
		}

		std::byte* pNext = mP_Data + m_CurrentByteOffset;

		CMAllocHandle<Ty> handle;

		handle.pData = new (pNext) Ty(std::forward<Args>(args)...);

		handle.MetaDataIndex = m_AllocData.size();
		handle.Result = CMAllocType::SUCCEEDED;

		m_AllocData.emplace_back(sizeof(Ty), nextOffset);
		return handle;
	}
}