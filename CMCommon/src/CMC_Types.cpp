#include "CMC_Types.hpp"

namespace CMCommon
{
	[[nodiscard]] bool CMFloat2::IsNearEqual(CMFloat2 other, float epsilon) const noexcept
	{
		return IsNearEqualFloat(x, other.x, epsilon) &&
			IsNearEqualFloat(y, other.y, epsilon);
	}

	[[nodiscard]] bool CMFloat3::IsNearEqual(CMFloat3 other, float epsilon) const noexcept
	{
		return IsNearEqualFloat(x, other.x, epsilon) &&
			IsNearEqualFloat(y, other.y, epsilon) &&
			IsNearEqualFloat(z, other.z, epsilon);
	}

	[[nodiscard]] bool CMTransform::IsNearEqual(const CMTransform& otherRef, float epsilon) const noexcept
	{
		return Scaling.IsNearEqual(otherRef.Scaling, epsilon) &&
			Rotation.IsNearEqual(otherRef.Rotation, epsilon) &&
			Translation.IsNearEqual(otherRef.Translation, epsilon);
	}

	[[nodiscard]] bool CMRigidTransform::IsNearEqual(const CMRigidTransform& otherRef, float epsilon) const noexcept
	{
		return Rotation.IsNearEqual(otherRef.Rotation, epsilon) &&
			Translation.IsNearEqual(otherRef.Translation, epsilon);
	}
}