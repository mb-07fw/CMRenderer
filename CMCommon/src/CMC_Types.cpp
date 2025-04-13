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


	CMFloat3::CMFloat3(float x, float y, float z) noexcept
		: x(x), y(y), z(z)
	{
	}

	CMFloat3::CMFloat3(CMFloat2 float2, float z) noexcept
		: x(float2.x), y(float2.y), z(z)
	{
	}
}