#pragma once

#include <cstdint>
#include <cmath>

namespace CMCommon
{
	inline constexpr float G_CMFLOAT_NEAR_EQUAL_EPSILON = 1e-4f;

	inline [[nodiscard]] bool IsNearEqualFloat(float x, float other, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept
	{
		return std::abs(x - other) <= epsilon;
	}
	
	struct CMFloat3;

	struct CMFloat2
	{
		CMFloat2(float x, float y) noexcept;
		CMFloat2(CMFloat3 float3) noexcept;

		CMFloat2() = default;
		~CMFloat2() = default;

		[[nodiscard]] bool operator==(CMFloat2 other) const noexcept;
		[[nodiscard]] bool IsEqual(CMFloat2 other) const noexcept;
		[[nodiscard]] bool IsNearEqual(CMFloat2 other, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) const noexcept;

		float x = 0;
		float y = 0;
	};

	struct CMFloat3
	{
		CMFloat3(float x, float y, float z) noexcept;
		CMFloat3(CMFloat2 float2, float z = 0.0f) noexcept;

		CMFloat3() = default;
		~CMFloat3() = default;

		[[nodiscard]] bool operator==(CMFloat3 other) const noexcept;
		[[nodiscard]] bool IsEqual(CMFloat3 other) const noexcept;
		[[nodiscard]] bool IsNearEqual(CMFloat3 other, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) const noexcept;

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};

	struct NormColor {
		float rgba[4];
	};

	struct CMTransform
	{
		CMTransform(
			CMFloat3 scaling = CMFloat3(1.0f, 1.0f, 1.0f),
			CMFloat3 rotation = CMFloat3(), 
			CMFloat3 translation = CMFloat3()
		) noexcept;

		~CMTransform() = default;

		[[nodiscard]] bool operator==(const CMTransform& otherRef) const noexcept;
		[[nodiscard]] bool IsEqual(const CMTransform& otherRef) const noexcept;
		[[nodiscard]] bool IsNearEqual(const CMTransform& otherRef, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) const noexcept;

		CMFloat3 Scaling;
		CMFloat3 Rotation;
		CMFloat3 Translation;
	};

	struct CMRigidTransform
	{
		CMRigidTransform(
			CMFloat3 rotation = CMFloat3(),
			CMFloat3 translation = CMFloat3()
		) noexcept;

		~CMRigidTransform() = default;

		[[nodiscard]] bool operator==(const CMRigidTransform& otherRef) const noexcept;
		[[nodiscard]] bool IsEqual(const CMRigidTransform& otherRef) const noexcept;
		[[nodiscard]] bool IsNearEqual(const CMRigidTransform& otherRef, float epsilon = G_CMFLOAT_NEAR_EQUAL_EPSILON) const noexcept;

		CMFloat3 Rotation;
		CMFloat3 Translation;
	};
}