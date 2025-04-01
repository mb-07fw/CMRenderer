#pragma once

#include <span>
#include <vector>
#include <type_traits>

namespace CMRenderer::Utility
{
	// An array which determines it's size at runtime, but cannot be modified after being set.
	template <typename DataTy>
	class CMStaticArray
	{
	public:
		inline CMStaticArray(std::span<DataTy> data) noexcept;
		CMStaticArray() = default;

		~CMStaticArray() = default;
	public:
		inline void Set(std::span<DataTy> data) noexcept;

		inline [[nodiscard]] const std::vector<DataTy>& Vector() const noexcept { return m_Data; }
		inline [[nodiscard]] const DataTy* Data() const noexcept { return m_Data.data(); }
		inline [[nodiscard]] size_t Size() const noexcept { return m_Data.size(); }
	private:
		inline void CopyData(std::span<DataTy> data) noexcept;
	private:
		std::vector<DataTy> m_Data;
	};

	template <typename DataTy>
	inline CMStaticArray<DataTy>::CMStaticArray(std::span<DataTy> data) noexcept
	{
		m_Data.reserve(data.size());

		CopyData(data);
	}

	template <typename DataTy>
	inline void CMStaticArray<DataTy>::Set(std::span<DataTy> data) noexcept
	{
		m_Data.clear();
		m_Data.reserve(data.size());

		CopyData(data);
	}

	template <typename DataTy>
	inline void CMStaticArray<DataTy>::CopyData(std::span<DataTy> data) noexcept
	{
		m_Data.assign(data.begin(), data.end());
	}
}