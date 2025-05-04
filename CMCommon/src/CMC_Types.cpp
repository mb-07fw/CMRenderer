#include "CMC_Types.hpp"

namespace CMCommon
{
	CMFloat2::CMFloat2(float x, float y) noexcept
		: x(x), y(y)
	{
	}

	CMFloat2::CMFloat2(CMFloat3 float3) noexcept
		: x(float3.x), y(float3.y)
	{
	}

	[[nodiscard]] bool CMFloat2::operator==(CMFloat2 other) const noexcept
	{
		return IsEqual(other);
	}

	[[nodiscard]] bool CMFloat2::IsEqual(CMFloat2 other) const noexcept
	{
		return x == other.x &&
			y == other.y;
	}

	[[nodiscard]] bool CMFloat2::IsNearEqual(CMFloat2 other, float epsilon) const noexcept
	{
		return IsNearEqualFloat(x, other.x, epsilon) &&
			IsNearEqualFloat(y, other.y, epsilon);
	}

	CMFloat3::CMFloat3(float x, float y, float z) noexcept
		: x(x), y(y), z(z)
	{
	}

	CMFloat3::CMFloat3(CMFloat2 float2, float z) noexcept
		: x(float2.x), y(float2.y), z(z)
	{
	}

	[[nodiscard]] bool CMFloat3::operator==(CMFloat3 other) const noexcept
	{
		return IsEqual(other);
	}

	[[nodiscard]] bool CMFloat3::IsEqual(CMFloat3 other) const noexcept
	{
		return x == other.x &&
			y == other.y &&
			z == other.z;
	}

	[[nodiscard]] bool CMFloat3::IsNearEqual(CMFloat3 other, float epsilon) const noexcept
	{
		return IsNearEqualFloat(x, other.x, epsilon) &&
			IsNearEqualFloat(y, other.y, epsilon) &&
			IsNearEqualFloat(z, other.z, epsilon);
	}

	CMTransform::CMTransform(CMFloat3 scaling, CMFloat3 rotation, CMFloat3 translation) noexcept
		: Scaling(scaling),
		  Rotation(rotation),
		  Translation(translation)
	{
	}

	[[nodiscard]] bool CMTransform::operator==(const CMTransform& other) const noexcept
	{
		return IsEqual(other);
	}

	[[nodiscard]] bool CMTransform::IsEqual(const CMTransform& other) const noexcept
	{
		return Scaling == other.Scaling &&
			Rotation == other.Rotation &&
			Translation == other.Translation;
	}

	[[nodiscard]] bool CMTransform::IsNearEqual(const CMTransform& otherRef, float epsilon) const noexcept
	{
		return Scaling.IsNearEqual(otherRef.Scaling, epsilon) &&
			Rotation.IsNearEqual(otherRef.Rotation, epsilon) &&
			Translation.IsNearEqual(otherRef.Translation, epsilon);
	}

	CMRigidTransform::CMRigidTransform(CMFloat3 rotation, CMFloat3 translation) noexcept
		: Rotation(rotation),
		  Translation(translation)
	{
	}

	[[nodiscard]] bool CMRigidTransform::operator==(const CMRigidTransform& other) const noexcept
	{
		return IsEqual(other);
	}

	[[nodiscard]] bool CMRigidTransform::IsEqual(const CMRigidTransform& other) const noexcept
	{
		return Rotation == other.Rotation &&
			Translation == other.Translation;
	}

	[[nodiscard]] bool CMRigidTransform::IsNearEqual(const CMRigidTransform& otherRef, float epsilon) const noexcept
	{
		return Rotation.IsNearEqual(otherRef.Rotation, epsilon) &&
			Translation.IsNearEqual(otherRef.Translation, epsilon);
	}

}