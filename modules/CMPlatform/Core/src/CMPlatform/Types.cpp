#include "CMPlatform/Types.hpp"

namespace CMEngine::Platform
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
}