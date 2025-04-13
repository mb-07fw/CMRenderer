#pragma once

#include <cstdint>

namespace CMRenderer
{
	struct CMFloat3;

	struct CMFloat2
	{
		CMFloat2(float x, float y) noexcept;
		CMFloat2(CMFloat3 float3) noexcept;
		
		CMFloat2() = default;
		~CMFloat2() = default;

		float x = 0;
		float y = 0;
	};

	struct CMFloat3
	{
		CMFloat3(float x, float y, float z) noexcept;
		CMFloat3(CMFloat2 float2, float z = 0) noexcept;

		CMFloat3() = default;
		~CMFloat3() = default;

		float x = 0;
		float y = 0;
		float z = 0;
	};

	template <typename Ty>
	struct CMViewPtr
	{
		inline CMViewPtr(Ty* ptr) noexcept;

		CMViewPtr() = default;
		~CMViewPtr() = default;

		inline [[nodiscard]] Ty* operator->() noexcept;
		inline [[nodiscard]] const Ty* operator->() const noexcept;
		inline [[nodiscard]] Ty& operator*() noexcept;
		inline [[nodiscard]] const Ty& operator*() const noexcept;
		inline void operator=(Ty* ptr) noexcept;

		inline [[nodiscard]] bool IsNull() const noexcept { return Ptr == nullptr; }

		Ty* Ptr = nullptr;
	};

	template <typename Ty>
	inline CMViewPtr<Ty>::CMViewPtr(Ty* ptr) noexcept
		: Ptr(ptr)
	{
	}

	template <typename Ty>
	inline [[nodiscard]] Ty* CMViewPtr<Ty>::operator->() noexcept
	{
		return Ptr;
	}

	template <typename Ty>
	inline [[nodiscard]] const Ty* CMViewPtr<Ty>::operator->() const noexcept
	{
		return Ptr;
	}

	template <typename Ty>
	inline [[nodiscard]] Ty& CMViewPtr<Ty>::operator*() noexcept
	{
		return *Ptr;
	}

	template <typename Ty>
	inline [[nodiscard]] const Ty& CMViewPtr<Ty>::operator*() const noexcept
	{
		return *Ptr;
	}

	template <typename Ty>
	inline void CMViewPtr<Ty>::operator=(Ty* ptr) noexcept
	{
		Ptr = ptr;
	}

}