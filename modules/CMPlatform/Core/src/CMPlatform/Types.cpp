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
}