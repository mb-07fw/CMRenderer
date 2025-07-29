#include "Common/Types.hpp"

namespace CMEngine::Common
{
	[[nodiscard]] bool Float2::IsNearEqual(Float2 other, float epsilon) const noexcept
	{
		return IsNearEqualFloat(x, other.x, epsilon) &&
			IsNearEqualFloat(y, other.y, epsilon);
	}

	[[nodiscard]] bool Float3::IsNearEqual(Float3 other, float epsilon) const noexcept
	{
		return IsNearEqualFloat(x, other.x, epsilon) &&
			IsNearEqualFloat(y, other.y, epsilon) &&
			IsNearEqualFloat(z, other.z, epsilon);
	}

	[[nodiscard]] bool Transform::IsNearEqual(const Transform& other, float epsilon) const noexcept
	{
		return Scaling.IsNearEqual(other.Scaling, epsilon) &&
			Rotation.IsNearEqual(other.Rotation, epsilon) &&
			Translation.IsNearEqual(other.Translation, epsilon);
	}

	[[nodiscard]] bool RigidTransform::IsNearEqual(const RigidTransform& other, float epsilon) const noexcept
	{
		return Rotation.IsNearEqual(other.Rotation, epsilon) &&
			Translation.IsNearEqual(other.Translation, epsilon);
	}
}