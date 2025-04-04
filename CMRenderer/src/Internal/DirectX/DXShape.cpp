#include "Core/CMPCH.hpp"
#include "Internal/DirectX/DXShape.hpp"
#include "Internal/DirectX/DXUtility.hpp"

namespace CMRenderer::CMDirectX
{
	CMCube::CMCube(uint32_t width, uint32_t height, uint32_t depth) noexcept
		: Width(width), Height(height), Depth(depth)
	{
	}

	CMNormCube::CMNormCube(CMCube cube, uint32_t width, uint32_t height) noexcept
	{
		
	}
}