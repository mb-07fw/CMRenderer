#pragma once

#include "CMR_Types.hpp"

namespace CMRenderer::CMDirectX
{
	struct CMRect 
	{
		CMRect(CMFloat2 pos) noexcept;

		CMRect() = default;
		~CMRect() = default;

		CMFloat2 Pos;
	};

	struct CMCube
	{
		CMCube(CMFloat3 pos) noexcept;

		CMCube() = default;
		~CMCube() = default;

		CMFloat3 Pos;
	};
}