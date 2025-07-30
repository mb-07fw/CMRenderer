#pragma once

#include <cstdint>
#include <cmath>
#include <limits>

namespace CMEngine::Common
{
	template <typename... Args>
	constexpr bool AllTriviallyCopyable = (std::is_trivially_copyable_v<Args>&&...);

	namespace Constants
	{
		inline constexpr uint32_t U32_MAX_VALUE = std::numeric_limits<uint32_t>::max();
		inline constexpr float FLOAT_MAX_VALUE = std::numeric_limits<float>::max();
	}

	struct NormColor {
		float rgba[4];
	};

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

		float x = 0;
		float y = 0;
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

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};
	
	struct Rect
	{
		inline constexpr Rect(float left, float right, float top, float bottom) noexcept;
		inline constexpr Rect(Float2 xBounds, Float2 yBounds) noexcept;

		Rect() = default;
		~Rect() = default;

		inline constexpr [[nodiscard]] float Width() const noexcept;
		inline constexpr [[nodiscard]] float Height() const noexcept;
		inline constexpr [[nodiscard]] Float2 XBounds() const noexcept;
		inline constexpr [[nodiscard]] Float2 YBounds() const noexcept;

		float Left = 0;
		float Right = 0;
		float Top = 0;
		float Bottom = 0;
	};

	struct Transform
	{
		inline constexpr Transform(
			Float3 scaling = Float3(1.0f, 1.0f, 1.0f),
			Float3 rotation = Float3(),
			Float3 translation = Float3()
		) noexcept;

		~Transform() = default;

		inline static constexpr [[nodiscard]] Transform Scale(Float3 scale) noexcept;
		inline static constexpr [[nodiscard]] Transform Rotate(Float3 rotate) noexcept;
		inline static constexpr [[nodiscard]] Transform Translate(Float3 translation) noexcept;

		inline constexpr [[nodiscard]] bool operator==(const Transform& other) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(const Transform& other) const noexcept;
		[[nodiscard]] bool IsNearEqual(const Transform& other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;

		Float3 Scaling;
		Float3 Rotation;
		Float3 Translation;
	};

	struct RigidTransform
	{
		inline constexpr RigidTransform(
			Float3 rotation = Float3(),
			Float3 translation = Float3()
		) noexcept;

		~RigidTransform() = default;

		inline static constexpr [[nodiscard]] RigidTransform Rotate(Float3 rotate) noexcept;
		inline static constexpr [[nodiscard]] RigidTransform Translate(Float3 translation) noexcept;

		inline constexpr [[nodiscard]] bool operator==(const RigidTransform& other) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(const RigidTransform& other) const noexcept;
		[[nodiscard]] bool IsNearEqual(const RigidTransform& other, float epsilon = G_NEAR_EQUAL_FLOAT_EPSILON) const noexcept;

		Float3 Rotation;
		Float3 Translation;
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

	inline constexpr Rect::Rect(float left, float right, float top, float bottom) noexcept
		: Left(left),
		  Right(right),
		  Top(top),
		  Bottom(bottom)
	{
	}

	inline constexpr Rect::Rect(Float2 xBounds, Float2 yBounds) noexcept
		: Left(xBounds.x),
		  Right(xBounds.y),
		  Top(yBounds.x),
		  Bottom(yBounds.y)
	{
	}

	inline constexpr [[nodiscard]] float Rect::Width() const noexcept
	{
		return Right - Left;
	}

	inline constexpr [[nodiscard]] float Rect::Height() const noexcept
	{
		return Bottom - Top;
	}

	inline constexpr [[nodiscard]] Float2 Rect::XBounds() const noexcept
	{
		return Float2(Left, Right);
	}

	inline constexpr [[nodiscard]] Float2 Rect::YBounds() const noexcept
	{
		return Float2(Top, Bottom);
	}

	inline constexpr Transform::Transform(Float3 scaling, Float3 rotation, Float3 translation) noexcept
		: Scaling(scaling),
		  Rotation(rotation),
		  Translation(translation)
	{
	}

	inline constexpr [[nodiscard]] Transform Transform::Scale(Float3 scale) noexcept
	{
		return Transform(scale);
	}

	inline constexpr [[nodiscard]] Transform Transform::Rotate(Float3 rotate) noexcept
	{
		return Transform(
			Float3(1.0f, 1.0f, 1.0f),
			rotate
		);
	}

	inline constexpr [[nodiscard]] Transform Transform::Translate(Float3 translation) noexcept
	{
		return Transform(
			Float3(1.0f, 1.0f, 1.0f),
			Float3(),
			translation
		);
	}

	inline constexpr [[nodiscard]] bool Transform::operator==(const Transform& other) const noexcept
	{
		return IsEqual(other);
	}

	inline constexpr [[nodiscard]] bool Transform::IsEqual(const Transform& other) const noexcept
	{
		return Scaling == other.Scaling &&
			Rotation == other.Rotation &&
			Translation == other.Translation;
	}

	inline constexpr RigidTransform::RigidTransform(Float3 rotation, Float3 translation) noexcept
		: Rotation(rotation),
		  Translation(translation)
	{
	}

	inline constexpr [[nodiscard]] RigidTransform RigidTransform::Rotate(Float3 rotate) noexcept
	{
		return RigidTransform(rotate);
	}

	inline constexpr [[nodiscard]] RigidTransform RigidTransform::Translate(Float3 translation) noexcept
	{
		return RigidTransform(
			Float3(),
			translation
		);
	}

	inline constexpr [[nodiscard]] bool RigidTransform::operator==(const RigidTransform& other) const noexcept
	{
		return IsEqual(other);
	}

	inline constexpr [[nodiscard]] bool RigidTransform::IsEqual(const RigidTransform& other) const noexcept
	{
		return Rotation == other.Rotation &&
			Translation == other.Translation;
	}
}