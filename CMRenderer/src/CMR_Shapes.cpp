#include "CMR_PCH.hpp"
#include "CMR_Shapes.hpp"

namespace CMRenderer::CMDirectX
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