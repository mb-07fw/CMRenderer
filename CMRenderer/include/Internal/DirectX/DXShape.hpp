#pragma once

#include <cstdint>

namespace CMRenderer::CMDirectX
{
	struct CMRect 
	{
		uint32_t Left = 0;
		uint32_t Top = 0;
		uint32_t Right = 0;
		uint32_t Bottom = 0;
	};

	struct CMCube
	{
		CMCube(uint32_t width, uint32_t height, uint32_t depth) noexcept;

		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t Depth = 0;
	};

	struct CMNormCube
	{
		CMNormCube(CMCube cube, uint32_t width, uint32_t height) noexcept;

		float NormWidth = 0;
		float NormHeight = 0;
		float NormDepth = 0;
	};
}