#pragma once

#include <cstdint>
#include <cmath>

namespace CMCommon
{
	struct NormColor {
		float rgba[4];
	};

	inline constexpr float G_CMFLOAT_NEAR_EQUAL_EPSILON = 1e-4f;

	/* Note: Cannot be constexpr due to std::abs not being constexpr. */
	inline [[nodiscard]] bool IsNearEqualFloat(float x, float other, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept
	{
		return std::abs(x - other) <= epsilon;
	}
	
	struct CMFloat3;

	struct CMFloat2
	{
		inline constexpr CMFloat2(float x, float y) noexcept;
		inline constexpr CMFloat2(CMFloat3 float3) noexcept;

		CMFloat2() = default;
		~CMFloat2() = default;

		inline constexpr [[nodiscard]] bool operator==(CMFloat2 other) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(CMFloat2 other) const noexcept;
		[[nodiscard]] bool IsNearEqual(CMFloat2 other, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) const noexcept;

		float x = 0;
		float y = 0;
	};

	struct CMFloat3
	{
		inline constexpr CMFloat3(float x, float y, float z) noexcept;
		inline constexpr CMFloat3(CMFloat2 float2, float z = 0.0f) noexcept;

		CMFloat3() = default;
		~CMFloat3() = default;

		inline constexpr [[nodiscard]] bool operator==(CMFloat3 other) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(CMFloat3 other) const noexcept;
		[[nodiscard]] bool IsNearEqual(CMFloat3 other, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) const noexcept;

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};

	inline constexpr CMFloat2::CMFloat2(float x, float y) noexcept
		: x(x), y(y)
	{
	}

	inline constexpr CMFloat2::CMFloat2(CMFloat3 float3) noexcept
		: x(float3.x), y(float3.y)
	{
	}

	inline constexpr [[nodiscard]] bool CMFloat2::operator==(CMFloat2 other) const noexcept
	{
		return IsEqual(other);
	}

	inline constexpr [[nodiscard]] bool CMFloat2::IsEqual(CMFloat2 other) const noexcept
	{
		return x == other.x &&
			y == other.y;
	}

	inline constexpr CMFloat3::CMFloat3(float x, float y, float z) noexcept
		: x(x), y(y), z(z)
	{
	}

	inline constexpr CMFloat3::CMFloat3(CMFloat2 float2, float z) noexcept
		: x(float2.x), y(float2.y), z(z)
	{
	}

	inline constexpr [[nodiscard]] bool CMFloat3::operator==(CMFloat3 other) const noexcept
	{
		return IsEqual(other);
	}

	inline constexpr [[nodiscard]] bool CMFloat3::IsEqual(CMFloat3 other) const noexcept
	{
		return x == other.x &&
			y == other.y &&
			z == other.z;
	}

	struct CMTransform
	{
		inline constexpr CMTransform(
			CMFloat3 scaling = CMFloat3(1.0f, 1.0f, 1.0f),
			CMFloat3 rotation = CMFloat3(), 
			CMFloat3 translation = CMFloat3()
		) noexcept;

		~CMTransform() = default;

		inline constexpr [[nodiscard]] bool operator==(const CMTransform& otherRef) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(const CMTransform& otherRef) const noexcept;
		[[nodiscard]] bool IsNearEqual(const CMTransform& otherRef, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) const noexcept;

		CMFloat3 Scaling;
		CMFloat3 Rotation;
		CMFloat3 Translation;
	};

	inline constexpr CMTransform::CMTransform(CMFloat3 scaling, CMFloat3 rotation, CMFloat3 translation) noexcept
		: Scaling(scaling),
		  Rotation(rotation),
		  Translation(translation)
	{
	}

	inline constexpr [[nodiscard]] bool CMTransform::operator==(const CMTransform& other) const noexcept
	{
		return IsEqual(other);
	}

	inline constexpr [[nodiscard]] bool CMTransform::IsEqual(const CMTransform& other) const noexcept
	{
		return Scaling == other.Scaling &&
			Rotation == other.Rotation &&
			Translation == other.Translation;
	}

	struct CMRigidTransform
	{
		inline constexpr CMRigidTransform(
			CMFloat3 rotation = CMFloat3(),
			CMFloat3 translation = CMFloat3()
		) noexcept;

		~CMRigidTransform() = default;

		inline constexpr [[nodiscard]] bool operator==(const CMRigidTransform& otherRef) const noexcept;
		inline constexpr [[nodiscard]] bool IsEqual(const CMRigidTransform& otherRef) const noexcept;
		[[nodiscard]] bool IsNearEqual(const CMRigidTransform& otherRef, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) const noexcept;

		CMFloat3 Rotation;
		CMFloat3 Translation;
	};

	inline constexpr CMRigidTransform::CMRigidTransform(CMFloat3 rotation, CMFloat3 translation) noexcept
		: Rotation(rotation),
		Translation(translation)
	{
	}

	inline constexpr [[nodiscard]] bool CMRigidTransform::operator==(const CMRigidTransform& other) const noexcept
	{
		return IsEqual(other);
	}

	inline constexpr [[nodiscard]] bool CMRigidTransform::IsEqual(const CMRigidTransform& other) const noexcept
	{
		return Rotation == other.Rotation &&
			Translation == other.Translation;
	}
}