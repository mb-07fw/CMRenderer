#include "CMC_Shapes.hpp"

namespace CMCommon
{
	CMRect::CMRect(CMFloat2 pos) noexcept
		: Pos(pos)
	{
	}

	CMCube::CMCube(CMFloat3 pos) noexcept
		: Pos(pos)
	{
	}
}