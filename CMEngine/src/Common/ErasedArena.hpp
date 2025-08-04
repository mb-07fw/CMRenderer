#pragma once

#include <cstdint>
#include <span>

namespace CMEngine::Common
{
	class ErasedArena
	{
	public:
		ErasedArena() = default;
		~ErasedArena() noexcept;
	public:
		/* Allocates @numBytes of memory.
		 * Returns true if the allocation succeeds. */
		[[nodiscard]] bool Allocate(size_t numBytes) noexcept;

		/* Deletes any allocated memory. */
		void Deallocate() noexcept;

		/* Re-allocates @numBytes + current size of memory if it exceeds the current byte size. 
		 * Returns true if the allocation succeeds. */
		bool ReserveNew(size_t numBytes) noexcept;

		/* Copies @data into memory at the current byte offset. 
		 * Returns true if enough memory was allocated previously,
		     and if the current byte size encompasses the current
			 byte offset + @data.size_bytes().
		 * Sets @outByteOffset to the byte offset of the copied data if the operation succeeds;
		 *   the current byte size otherwise. */
		bool Copy(std::span<const std::byte> data, size_t& outByteOffset) noexcept;

		inline [[nodiscard]] std::byte* Data() const noexcept { return mP_Data; }
		inline [[nodiscard]] bool HasAllocated() const noexcept { return mP_Data != nullptr; }
	private:
		size_t m_CurrentByteSize = 0;
		size_t m_CurrentByteOffset = 0;
		std::byte* mP_Data = nullptr;
	};
}