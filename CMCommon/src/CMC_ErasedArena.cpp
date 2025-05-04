#include "CMC_ErasedArena.hpp"

namespace CMCommon
{
	CMErasedArena::CMErasedArena(size_t bytes) noexcept
		: m_CurrentByteSize(bytes)
	{
		mP_Memory = operator new(bytes);
	}

	CMErasedArena::~CMErasedArena() noexcept
	{
		if (mP_Memory != nullptr)
			operator delete(mP_Memory);
	}
}