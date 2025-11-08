#include "PCH.hpp"
#include "Buffer.hpp"
#include "Macros.hpp"

namespace CMEngine
{
	Buffer::~Buffer() noexcept
	{
		if (HasAllocated())
			Dealloc();
	}

	void Buffer::Alloc(size_t numBytes) noexcept
	{
		CM_ENGINE_ASSERT(mP_Data == nullptr);

		mP_Data = (std::byte*)operator new(numBytes);
		m_Size = numBytes;
	}

	void Buffer::Dealloc() noexcept
	{
		CM_ENGINE_ASSERT(mP_Data != nullptr);

		delete[] mP_Data;

		mP_Data = nullptr;
		m_Size = 0;
	}

	void Buffer::Realloc(size_t numBytes) noexcept
	{
		if (numBytes <= m_Size)
			return;

		CM_ENGINE_ASSERT(mP_Data != nullptr);

		void* pNewBlock = std::realloc(Erased(), numBytes);

		if (pNewBlock != nullptr)
		{
			mP_Data = (std::byte*)pNewBlock;
			m_Size = numBytes;
			return;
		}

		/* realloc failed, manually copy data to new block. */
		pNewBlock = operator new(numBytes);
		std::memcpy(pNewBlock, mP_Data, m_Size);

		Dealloc();

		mP_Data = (std::byte*)pNewBlock;
		m_Size = numBytes;

		return;
	}

	void Buffer::Reserve(size_t numBytes) noexcept
	{
		if (!HasAllocated())
			return;

		Realloc(numBytes);
	}

	void Buffer::Grow(size_t additionalBytes) noexcept
	{
		if (!HasAllocated())
			return;

		Realloc(m_Size + additionalBytes);
	}
}