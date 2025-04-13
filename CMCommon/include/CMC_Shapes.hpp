#pragma once

#include "CMC_Types.hpp"

namespace CMCommon
{
	enum class CMShapeType : uint8_t {
		RECT, CUBE
	};

	struct CMRect
	{
		CMRect(CMFloat2 pos) noexcept;
		CMRect(CMRect&) = default;

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