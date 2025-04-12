#include "Core/CMPCH.hpp"
#include "Internal/CMShapes.hpp"

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