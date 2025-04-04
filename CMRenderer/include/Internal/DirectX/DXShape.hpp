#pragma once

#include <cstdint>

#include <DirectXMath.h>

namespace CMRenderer::CMDirectX
{
	struct DXRect 
	{
		uint32_t Left = 0;
		uint32_t Top = 0;
		uint32_t Right = 0;
		uint32_t Bottom = 0;
	};

	struct DXCube
	{
		DXCube() = default;
		DXCube(DirectX::XMFLOAT3 pos) noexcept;

		DirectX::XMFLOAT3 Pos;
	};

	/*struct CMNormCube
	{
		CMNormCube(CMCube cube, uint32_t width, uint32_t height) noexcept;

		float NormWidth = 0;
		float NormHeight = 0;
		float NormDepth = 0;
	};*/
}