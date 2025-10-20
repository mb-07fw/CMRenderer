#include "Types.hpp"

namespace CMEngine
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

	[[nodiscard]] bool Rect::IsNearEqual(Rect other, float epsilon) const noexcept
	{
		return IsNearEqualFloat(left, other.left, epsilon) &&
			IsNearEqualFloat(top, other.top, epsilon) &&
			IsNearEqualFloat(right, other.right, epsilon) &&
			IsNearEqualFloat(bottom, other.bottom, epsilon);
	}

	[[nodiscard]] bool Transform::IsNearEqual(const Transform& other, float epsilon) const noexcept
	{
		return Translation.IsNearEqual(other.Translation, epsilon) &&
			Scaling.IsNearEqual(other.Scaling, epsilon) &&
			Rotation.IsNearEqual(other.Rotation, epsilon);
	}

	[[nodiscard]] bool RigidTransform::IsNearEqual(const RigidTransform& other, float epsilon) const noexcept
	{
		return Translation.IsNearEqual(other.Translation, epsilon) &&
			Rotation.IsNearEqual(other.Rotation, epsilon);
	}
}