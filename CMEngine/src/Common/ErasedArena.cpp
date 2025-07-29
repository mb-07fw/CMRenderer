#include "Common/ErasedArena.hpp"

namespace CMEngine::Common
{
	ErasedArena::ErasedArena(size_t bytes) noexcept
		: m_CurrentByteSize(bytes)
	{
		mP_Memory = operator new(bytes);
	}

	ErasedArena::~ErasedArena() noexcept
	{
		if (mP_Memory != nullptr)
			operator delete(mP_Memory);
	}
}