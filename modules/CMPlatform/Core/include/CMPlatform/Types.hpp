#pragma once

#include <cstdint>
#include <cmath>
#include <limits>

namespace CMEngine::Platform
{
	template <typename... Args>
	constexpr bool AllTriviallyCopyable = (std::is_trivially_copyable_v<Args>&&...);

	inline constexpr float G_NEAR_EQUAL_FLOAT_EPSILON = 1e-4f;

	/* Note: Cannot be constexpr due to std::abs not being constexpr. */
	inline [[nodiscard]] bool IsNearEqualFloat(float x, float other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) noexcept
	{
		return std::abs(x - other) <= epsilon;
	}

	struct Float3;

	struct Float2
	{
		inline constexpr Float2(float x, float y) noexcept;
		inline constexpr Float2(Float3 float3) noexcept;

		Float2() = default;
		~Float2() = default;

		inline constexpr [[nodiscard]] bool operator==(Float2 other) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(Float2 other) const noexcept;
		[[nodiscard]] bool IsNearEqual(Float2 other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;

		float x = 0.0f, y = 0.0f;
	};

	struct Float3
	{
		inline constexpr Float3(float x, float y, float z = 0.0f) noexcept;
		inline constexpr Float3(Float2 float2, float z = 0.0f) noexcept;

		Float3() = default;
		~Float3() = default;

		inline constexpr [[nodiscard]] bool operator==(Float3 other) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(Float3 other) const noexcept;
		[[nodiscard]] bool IsNearEqual(Float3 other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;
		inline constexpr [[nodiscard]] bool IsZero() const noexcept;

		float x = 0.0f, y = 0.0f, z = 0.0f;
	};

	inline constexpr Float2::Float2(float x, float y) noexcept
		: x(x), y(y)
	{
	}

	inline constexpr Float2::Float2(Float3 float3) noexcept
		: x(float3.x), y(float3.y)
	{
	}

	inline constexpr [[nodiscard]] bool Float2::operator==(Float2 other) const noexcept
	{
		return IsEqual(other);
	}

	inline constexpr [[nodiscard]] bool Float2::IsEqual(Float2 other) const noexcept
	{
		return x == other.x &&
			y == other.y;
	}

	inline constexpr Float3::Float3(float x, float y, float z) noexcept
		: x(x), y(y), z(z)
	{
	}

	inline constexpr Float3::Float3(Float2 float2, float z) noexcept
		: x(float2.x), y(float2.y), z(z)
	{
	}

	inline constexpr [[nodiscard]] bool Float3::operator==(Float3 other) const noexcept
	{
		return IsEqual(other);
	}

	inline constexpr [[nodiscard]] bool Float3::IsEqual(Float3 other) const noexcept
	{
		return x == other.x &&
			y == other.y &&
			z == other.z;
	}

	inline constexpr [[nodiscard]] bool Float3::IsZero() const noexcept
	{
		return x == 0 &&
			y == 0 &&
			z == 0;
	}
}