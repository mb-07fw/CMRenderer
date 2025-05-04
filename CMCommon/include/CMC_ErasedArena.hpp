#pragma once

namespace CMCommon
{
	class CMErasedArena
	{
	public:
		CMErasedArena(size_t bytes) noexcept;
		CMErasedArena() = default;
		~CMErasedArena() noexcept;
	public:
		inline [[nodiscard]] void* Memory() const noexcept { return mP_Memory; }
		inline [[nodiscard]] bool HasAllocated() const noexcept { return mP_Memory == nullptr; }
	private:
		size_t m_CurrentByteSize = 0;
		size_t m_CurrentByteOffset = 0;
		void* mP_Memory = nullptr;
	};
}