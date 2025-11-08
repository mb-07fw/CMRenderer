#pragma once

#include <memory>
#include <span>

namespace CMEngine
{
	class Buffer
	{
	public:
	public:
		Buffer() = default;
		~Buffer() noexcept;
	public:
		void Alloc(size_t numBytes) noexcept;
		void Dealloc() noexcept;
		void Realloc(size_t numBytes) noexcept;
		void Reserve(size_t numBytes) noexcept;
		void Grow(size_t additionalBytes) noexcept;

		inline [[nodiscard]] bool HasAllocated() const noexcept { return mP_Data != nullptr; }

		inline [[nodiscard]] std::byte* Data() noexcept { return mP_Data; }
		inline [[nodiscard]] size_t Size() const noexcept { return m_Size; }
		inline [[nodiscard]] void* Erased() noexcept { return (void*)mP_Data; }
	protected:
		std::byte* mP_Data = nullptr;
		size_t m_Size = 0;
	};

	class SubBuffer : public Buffer
	{
	public:
		SubBuffer() = default;
		~SubBuffer() = default;
	public:
		template <typename Ty>
		inline void AppendData(std::span<Ty> data) noexcept;

		inline [[nodiscard]] size_t CurrentOffset() const noexcept { return m_CurrentOffset; }
	private:
		size_t m_CurrentOffset = 0;
	};

	template <typename Ty>
	inline void SubBuffer::AppendData(std::span<Ty> data) noexcept
	{
		constexpr size_t ArbitraryGrowthFactor = 2;

		size_t endOffset = m_CurrentOffset + data.size_bytes();

		if (!HasAllocated())
			Alloc(data.size_bytes() * ArbitraryGrowthFactor);
		else if (m_Size < endOffset)
			Realloc(endOffset * ArbitraryGrowthFactor);

		std::memcpy(mP_Data + m_CurrentOffset, data.data(), data.size_bytes());
		m_CurrentOffset += data.size_bytes();
	}
}