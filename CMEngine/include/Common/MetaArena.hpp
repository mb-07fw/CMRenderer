#pragma once

#include <vector>
#include <type_traits>
#include <cstdint>
#include <cstddef>

#include "Common/Logger.hpp"

namespace CMEngine::Common
{
	enum class AllocType : int8_t
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

	inline constexpr [[nodiscard]] int8_t AllocTypeToInt8(AllocType type) noexcept
	{
		return static_cast<int8_t>(type);
	}

	inline constexpr [[nodiscard]] bool AllocSucceeded(AllocType type) noexcept
	{
		return AllocTypeToInt8(type) > AllocTypeToInt8(AllocType::FAILED);
	}

	inline constexpr [[nodiscard]] bool AllocFailed(AllocType type) noexcept
	{
		return AllocTypeToInt8(type) < AllocTypeToInt8(AllocType::SUCCEEDED);
	}

	struct AllocMetaData
	{
		inline constexpr AllocMetaData(size_t bytes, size_t byteOffset) noexcept
			: Bytes(bytes),
			  ByteOffset(byteOffset)
		{
		}

		~AllocMetaData() = default;

		size_t Bytes = 0;
		size_t ByteOffset = 0;
	};

	template <typename Ty>
	struct AllocHandle
	{
		inline constexpr AllocHandle(size_t metaDataIndex, AllocType result, Ty* pData) noexcept
			: MetaDataIndex(MetaDataIndex),
			  Result(result),
			  pData(pData)
		{
		}

		~AllocHandle() = default;

		size_t MetaDataIndex = 0;
		AllocType Result = AllocType::INVALID; /* Note to self; AllocType::INVALID
												  * implies an un-initialized alloc handle. */
		Ty* pData = nullptr;
	};

	class MetaArena
	{
	public:
		MetaArena(LoggerWide& logger) noexcept;
		~MetaArena() noexcept;
	public:
		/* Allocates the initial block of memory for the arena.
		 * 
		 * Returns AllocType::SUCCEEDED if allocation succeeds;
		 *   any AllocType < 1 otherwise.
		 * 
		 * Use AllocSucceeded to check if returned AllocType is a 'succeeded' type.
		 */
		[[nodiscard]] AllocType Allocate(size_t bytes) noexcept;

		/* De-allocates the arena's block of memory.
		 * 
		 * Returns AllocType::SUCCEEDED if de-allocation succeeds;
		 *   any AllocType < 1 otherwise.
		 *
		 * Use AllocSucceeded to check if returned AllocType is a 'succeeded' type.
		 *
		 * NOTE: May cause UB for any objects not destroyed before de-allocation.
		 */
		AllocType Deallocate() noexcept;

		template <typename Ty, typename... Args>
			requires std::is_trivially_copyable_v<Ty>
		inline [[nodiscard]] AllocHandle<Ty> ConstructNext(Args&&... args) noexcept;

		inline [[nodiscard]] size_t TotalAllocations() const noexcept { return m_AllocData.size(); }
	private:
		static constexpr size_t S_ALLOC_BYTE_SIZE_SANITY_BOUNDARY = 10'000;
		static constexpr size_t S_INVALID_META_DATA_INDEX = static_cast<size_t>(-1);
		std::byte* mP_Data = nullptr;
		size_t m_CurrentByteOffset = 0;
		size_t m_TotalBytes = 0;
		bool m_AllocatedBlock = false;
		std::vector<AllocMetaData> m_AllocData;
		LoggerWide& m_Logger;
	};

	template <typename Ty, typename... Args>
		requires std::is_trivially_copyable_v<Ty>
	inline [[nodiscard]] AllocHandle<Ty> MetaArena::ConstructNext(Args&&... args) noexcept
	{
		constexpr std::wstring_view FuncTag = L"MetaArena [ConstructNext] | ";

		if (!m_AllocatedBlock)
		{
			m_Logger.LogWarningNLTagged(FuncTag, L"Arena hasn't allocated a block of memory yet.");
			return AllocType::FAILED_NOT_ALLOCATED;
		}

		size_t nextOffset = m_CurrentByteOffset + sizeof(Ty);

		if (nextOffset > m_TotalBytes)
		{
			m_Logger.LogWarningNLFormatted(
				FuncTag,
				L"Allocation of size `{}` would overflow current byte offset of `{}`.",
				sizeof(Ty), m_CurrentByteOffset
			);

			return AllocHandle<Ty>(
				S_INVALID_META_DATA_INDEX,
				AllocType::FAILED_BYTE_SIZE_OVERFLOWING_ARENA,
				nullptr
			);
		}

		std::byte* pNext = mP_Data + m_CurrentByteOffset;

		AllocHandle<Ty> handle;

		handle.pData = new (pNext) Ty(std::forward<Args>(args)...);

		handle.MetaDataIndex = m_AllocData.size();
		handle.Result = AllocType::SUCCEEDED;

		m_AllocData.emplace_back(sizeof(Ty), nextOffset);
		return handle;
	}
}