#include "Common/MetaArena.hpp"

namespace CMEngine::Common
{
	MetaArena::MetaArena(LoggerWide& logger) noexcept
		: m_Logger(logger)
	{
	}

	MetaArena::~MetaArena() noexcept
	{
		if (m_AllocatedBlock)
			Deallocate();
	}

	[[nodiscard]] AllocType MetaArena::Allocate(size_t bytes) noexcept
	{
		constexpr std::wstring_view FuncTag = L"MetaArena [Allocate] | ";

		if (m_AllocatedBlock)
		{
			m_Logger.LogWarningNLTagged(
				FuncTag,
				L"Arena already allocated a block of memory."
			);

			return AllocType::FAILED_ALREADY_ALLOCATED;
		}
		else if (bytes >= S_ALLOC_BYTE_SIZE_SANITY_BOUNDARY)
		{
			m_Logger.LogWarningNLFormatted(
				FuncTag,
				L"Allocation size exceeds sanity limit : `{}`.",
				S_ALLOC_BYTE_SIZE_SANITY_BOUNDARY
			);

			return AllocType::FAILED_ALLOC_INSANE_SIZE;
		}

		mP_Data = reinterpret_cast<std::byte*>(operator new(bytes, std::nothrow));

		if (mP_Data == nullptr)
			return AllocType::FAILED_BAD_ALLOC;

		m_TotalBytes = bytes;
		m_AllocatedBlock = true;
		return AllocType::SUCCEEDED;
	}

	AllocType MetaArena::Deallocate() noexcept
	{
		constexpr std::wstring_view FuncTag = L"MetaArena [Deallocate] | ";

		if (!m_AllocatedBlock)
		{
			m_Logger.LogWarningNLTagged(
				FuncTag,
				L"Arena doesn't have a block of memory to de-allocate."
			);
				
			return AllocType::FAILED_NOT_ALLOCATED;
		}

		operator delete(mP_Data);

		m_TotalBytes = 0;
		m_CurrentByteOffset = 0;
		m_AllocatedBlock = false;

		return AllocType::SUCCEEDED;
	}
}