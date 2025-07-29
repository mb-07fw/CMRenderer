#pragma once

#include <cstdint>

namespace CMEngine::Common
{
	class ErasedArena
	{
	public:
		ErasedArena(size_t bytes) noexcept;
		ErasedArena() = default;
		~ErasedArena() noexcept;
	public:
		inline [[nodiscard]] void* Memory() const noexcept { return mP_Memory; }
		inline [[nodiscard]] bool HasAllocated() const noexcept { return mP_Memory == nullptr; }
	private:
		size_t m_CurrentByteSize = 0;
		size_t m_CurrentByteOffset = 0;
		void* mP_Memory = nullptr;
	};
}