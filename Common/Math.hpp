#pragma once

#include "Cast.hpp"

#include <array>
#include <algorithm>
#include <type_traits>
#include <utility>

#include <cmath>

inline constexpr float G_NearEqualEpsilon_f = 1e-4f;

inline [[nodiscard]] bool IsNearEqual(float x, float other, float epsilon = G_NearEqualEpsilon_f) noexcept
{
	return std::abs(x - other) <= epsilon;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
struct Vector
{
	using Self = Vector<T, Size>;

	template <typename... Args>
		requires (sizeof...(Args) == Size)
	inline constexpr Vector(Args&&... args) noexcept
		: Elems{ Cast<T>(args)... }
	{
	}

	Vector() = default;
	~Vector() = default;

	Vector(const Self&) = default;
	Vector(Self&&) = default;
	Vector& operator=(const Self&) = default;
	Vector& operator=(Self&&) = default;

	inline constexpr T& x() noexcept { return Elems[0]; }
	inline constexpr const T& x() const noexcept { return Elems[0]; }

	inline constexpr T& y() noexcept
		requires (Size >= 2) { return Elems[1]; }

	inline constexpr const T& y() const noexcept
		requires (Size >= 2) { return Elems[1]; }

	inline constexpr T& z() noexcept
		requires (Size >= 3) { return Elems[2]; }

	inline constexpr const T& z() const noexcept
		requires (Size >= 3) { return Elems[2]; }

	inline constexpr T& operator[](size_t index) noexcept { return Elems.at(index); }
	inline constexpr const T& operator[](size_t index) const noexcept { return Elems.at(index); }

	inline constexpr [[nodiscard]] Self operator+(const Self& rhs) const noexcept { return Add(rhs); }
	inline constexpr [[nodiscard]] Self operator-(const Self& rhs) const noexcept { return Subtract(rhs); }
	inline constexpr [[nodiscard]] Self operator*(const Self& rhs) const noexcept { return Multiply(rhs); }
	inline constexpr [[nodiscard]] Self operator/(const Self& rhs) const noexcept { return Divide(rhs); }

	inline constexpr [[nodiscard]] Self operator+(const T& rhs) const noexcept { return Add(rhs); }
	inline constexpr [[nodiscard]] Self operator-(const T& rhs) const noexcept { return Subtract(rhs); }
	inline constexpr [[nodiscard]] Self operator*(const T& rhs) const noexcept { return Multiply(rhs); }
	inline constexpr [[nodiscard]] Self operator/(const T& rhs) const noexcept { return Divide(rhs); }

	inline constexpr Self& operator+=(const Self& rhs) noexcept;
	inline constexpr Self& operator-=(const Self& rhs) noexcept;
	inline constexpr Self& operator*=(const Self& rhs) noexcept;
	inline constexpr Self& operator/=(const Self& rhs) noexcept;

	inline constexpr Self& operator+=(const T& rhs) noexcept;
	inline constexpr Self& operator-=(const T& rhs) noexcept;
	inline constexpr Self& operator*=(const T& rhs) noexcept;
	inline constexpr Self& operator/=(const T& rhs) noexcept;

	inline constexpr [[nodiscard]] bool operator==(const Self& other) const noexcept { return IsEqual(other); }
	inline constexpr [[nodiscard]] bool operator==(const T& value) const noexcept { return IsEqual(value); }

	inline constexpr [[nodiscard]] bool IsEqual(const Self& other) const noexcept;
	inline constexpr [[nodiscard]] bool IsEqual(const T& value) const noexcept;

	inline constexpr [[nodiscard]] Self Add(const Self& other) const noexcept;
	inline constexpr [[nodiscard]] Self Add(const T& value) const noexcept;

	inline constexpr [[nodiscard]] Self Subtract(const Self& other) const noexcept;
	inline constexpr [[nodiscard]] Self Subtract(const T& value) const noexcept;

	inline constexpr [[nodiscard]] Self Multiply(const Self& other) const noexcept;
	inline constexpr [[nodiscard]] Self Multiply(const T& value) const noexcept;

	inline constexpr [[nodiscard]] Self Divide(const Self& other) const noexcept;
	inline constexpr [[nodiscard]] Self Divide(const T& value) const noexcept;

	inline [[nodiscard]] bool IsNearEqual(const Self& other, float epsilon = G_NearEqualEpsilon_f) const noexcept
		requires std::is_floating_point_v<T>;

	std::array<T, Size> Elems = {};
};


#pragma region Vector<T, Size>

#pragma region Operators
template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr Vector<T, Size>& Vector<T, Size>::operator+=(const Self& rhs) noexcept
{
	for (size_t i = 0; i < Size; ++i)
		Elems[i] = Elems[i] + rhs.Elems[i];

	return *this;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr Vector<T, Size>& Vector<T, Size>::operator-=(const Self& rhs) noexcept
{
	for (size_t i = 0; i < Size; ++i)
		Elems[i] = Elems[i] - rhs.Elems[i];

	return *this;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr Vector<T, Size>& Vector<T, Size>::operator*=(const Self& rhs) noexcept
{
	for (size_t i = 0; i < Size; ++i)
		Elems[i] = Elems[i] * rhs.Elems[i];

	return *this;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr Vector<T, Size>& Vector<T, Size>::operator/=(const Self& rhs) noexcept
{
	for (size_t i = 0; i < Size; ++i)
		Elems[i] = Elems[i] / rhs.Elems[i];

	return *this;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr Vector<T, Size>& Vector<T, Size>::operator+=(const T& rhs) noexcept
{
	for (size_t i = 0; i < Size; ++i)
		Elems[i] = Elems[i] + rhs;

	return *this;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr Vector<T, Size>& Vector<T, Size>::operator-=(const T& rhs) noexcept
{
	for (size_t i = 0; i < Size; ++i)
		Elems[i] = Elems[i] - rhs.Elems[i];

	return *this;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr Vector<T, Size>& Vector<T, Size>::operator*=(const T& rhs) noexcept
{
	for (size_t i = 0; i < Size; ++i)
		Elems[i] = Elems[i] * rhs.Elems[i];

	return *this;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr Vector<T, Size>& Vector<T, Size>::operator/=(const T& rhs) noexcept
{
	for (size_t i = 0; i < Size; ++i)
		Elems[i] = Elems[i] / rhs.Elems[i];

	return *this;
}
#pragma endregion

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] bool Vector<T, Size>::IsEqual(const Self& other) const noexcept
{
	for (size_t i = 0; i < Size; ++i)
		if (Elems[i] != other.Elems[i])
			return false;

	return true;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] bool Vector<T, Size>::IsEqual(const T& value) const noexcept
{
	for (size_t i = 0; i < Size; ++i)
		if (Elems[i] != value)
			return false;

	return true;
}

#pragma region Arithmetic
template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] Vector<T, Size> Vector<T, Size>::Add(const Self& other) const noexcept
{
	Vector<T, Size> sum;

	for (size_t i = 0; i < Size; ++i)
		sum[i] = Elems[i] + other.Elems[i];

	return sum;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] Vector<T, Size> Vector<T, Size>::Add(const T& value) const noexcept
{
	Vector<T, Size> sum;

	for (size_t i = 0; i < Size; ++i)
		sum[i] = Elems[i] + value;

	return sum;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] Vector<T, Size> Vector<T, Size>::Subtract(const Self& other) const noexcept
{
	Vector<T, Size> diff;

	for (size_t i = 0; i < Size; ++i)
		diff[i] = Elems[i] - other.Elems[i];

	return diff;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] Vector<T, Size> Vector<T, Size>::Subtract(const T& value) const noexcept
{
	Vector<T, Size> diff;

	for (size_t i = 0; i < Size; ++i)
		diff[i] = Elems[i] - value;

	return diff;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] Vector<T, Size> Vector<T, Size>::Multiply(const Self& other) const noexcept
{
	Vector<T, Size> prod;

	for (size_t i = 0; i < Size; ++i)
		prod[i] = Elems[i] * other.Elems[i];

	return prod;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] Vector<T, Size> Vector<T, Size>::Multiply(const T& value) const noexcept
{
	Vector<T, Size> prod;

	for (size_t i = 0; i < Size; ++i)
		prod[i] = Elems[i] * value;

	return prod;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] Vector<T, Size> Vector<T, Size>::Divide(const Self& other) const noexcept
{
	Vector<T, Size> quotient;

	for (size_t i = 0; i < Size; ++i)
		quotient[i] = Elems[i] / other.Elems[i];

	return quotient;
}

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline constexpr [[nodiscard]] Vector<T, Size> Vector<T, Size>::Divide(const T& value) const noexcept
{
	Vector<T, Size> quotient;

	for (size_t i = 0; i < Size; ++i)
		quotient[i] = Elems[i] / value;

	return quotient;
}

#pragma endregion

template <typename T, size_t Size>
	requires std::is_trivially_copyable_v<T>
inline [[nodiscard]] bool Vector<T, Size>::IsNearEqual(const Self& other, float epsilon) const noexcept
	requires std::is_floating_point_v<T>
{
	for (size_t i = 0; i < Size; ++i)
		if (!::IsNearEqual(Elems[i], other.Elems[i], epsilon))
			return false;

	return true;
}

#pragma endregion

using Float2 = Vector<float, 2>;
using Float3 = Vector<float, 3>;
using Float4 = Vector<float, 4>;

using UInt32_2 = Vector<uint32_t, 2>;
using UInt32_3 = Vector<uint32_t, 3>;
using UInt32_4 = Vector<uint32_t, 4>;