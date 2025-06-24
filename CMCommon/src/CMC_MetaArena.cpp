#include "CMC_MetaArena.hpp"

namespace CMCommon
{
	CMMetaArena::CMMetaArena(CMCommon::CMLoggerWide& logger) noexcept
		: m_Logger(logger)
	{
	}

	CMMetaArena::~CMMetaArena() noexcept
	{
		if (m_AllocatedBlock)
			Deallocate();
	}

	[[nodiscard]] CMAllocType CMMetaArena::Allocate(size_t bytes) noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMetaArena [Allocate] | ";

		if (m_AllocatedBlock)
		{
			m_Logger.LogWarningNLTagged(
				FuncTag,
				L"Arena already allocated a block of memory."
			);

			return CMAllocType::FAILED_ALREADY_ALLOCATED;
		}
		else if (bytes >= S_ALLOC_BYTE_SIZE_SANITY_BOUNDARY)
		{
			m_Logger.LogWarningNLFormatted(
				FuncTag,
				L"Allocation size exceeds sanity limit : `{}`.",
				S_ALLOC_BYTE_SIZE_SANITY_BOUNDARY
			);

			return CMAllocType::FAILED_ALLOC_INSANE_SIZE;
		}

		mP_Data = reinterpret_cast<std::byte*>(operator new(bytes, std::nothrow));

		if (mP_Data == nullptr)
			return CMAllocType::FAILED_BAD_ALLOC;

		m_TotalBytes = bytes;
		m_AllocatedBlock = true;
		return CMAllocType::SUCCEEDED;
	}

	CMAllocType CMMetaArena::Deallocate() noexcept
	{
		constexpr std::wstring_view FuncTag = L"CMMetaArena [Deallocate] | ";

		if (!m_AllocatedBlock)
		{
			m_Logger.LogWarningNLTagged(
				FuncTag,
				L"Arena doesn't have a block of memory to de-allocate."
			);
				
			return CMAllocType::FAILED_NOT_ALLOCATED;
		}

		operator delete(mP_Data);

		m_TotalBytes = 0;
		m_CurrentByteOffset = 0;
		m_AllocatedBlock = false;

		return CMAllocType::SUCCEEDED;
	}
}