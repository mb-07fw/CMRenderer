#include "Common/ErasedArena.hpp"

namespace CMEngine::Common
{
	ErasedArena::~ErasedArena() noexcept
	{
		Deallocate();
	}

	[[nodiscard]] bool ErasedArena::Allocate(size_t numBytes) noexcept
	{
		if (HasAllocated() && numBytes <= m_CurrentByteSize)
			return false;
		else if (HasAllocated())
			Deallocate();

		mP_Data = reinterpret_cast<std::byte*>(operator new(numBytes));

		if (mP_Data == nullptr)
			return false;

		m_CurrentByteSize = numBytes;
		return true;
	}

	void ErasedArena::Deallocate() noexcept
	{
		if (mP_Data != nullptr)
			operator delete(mP_Data);
		
		m_CurrentByteSize = 0;
		m_CurrentByteOffset = 0;
	}

	bool ErasedArena::ReserveNew(size_t numBytes) noexcept
	{
		return Allocate(numBytes + m_CurrentByteSize);
	}

	bool ErasedArena::Copy(std::span<const std::byte> data, size_t& outByteOffset) noexcept
	{
		outByteOffset = m_CurrentByteSize;

		if (!HasAllocated())
			return false;
		
		size_t numBytes = data.size_bytes();
		size_t endPos = m_CurrentByteOffset + numBytes;

		if (m_CurrentByteSize < endPos)
			return false;

		std::byte* pDest = mP_Data + m_CurrentByteOffset;
		std::copy(data.data(), data.data() + numBytes, pDest);

		outByteOffset = m_CurrentByteOffset;
		m_CurrentByteOffset += numBytes;
		return true;
	}
}