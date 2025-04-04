#pragma once

#include <d3dcommon.h>
#include <string_view>

#include <cstdint>

namespace CMRenderer::CMDirectX::DXUtility
{
	constexpr std::wstring_view D3DFeatureLevelToWStrView(D3D_FEATURE_LEVEL featureLevel) noexcept;
	
	constexpr std::wstring_view D3DFeatureLevelToWStrView(D3D_FEATURE_LEVEL featureLevel) noexcept
	{
		switch (featureLevel)
		{
		case D3D_FEATURE_LEVEL_1_0_CORE: return std::wstring_view(L"D3D_FEATURE_LEVEL_1_0_CORE");
		case D3D_FEATURE_LEVEL_9_1:		 return std::wstring_view(L"D3D_FEATURE_LEVEL_9_1");
		case D3D_FEATURE_LEVEL_9_2:		 return std::wstring_view(L"D3D_FEATURE_LEVEL_9_2");
		case D3D_FEATURE_LEVEL_9_3:		 return std::wstring_view(L"D3D_FEATURE_LEVEL_9_3");
		case D3D_FEATURE_LEVEL_10_0:	 return std::wstring_view(L"D3D_FEATURE_LEVEL_10_0");
		case D3D_FEATURE_LEVEL_10_1:	 return std::wstring_view(L"D3D_FEATURE_LEVEL_10_1");
		case D3D_FEATURE_LEVEL_11_0:	 return std::wstring_view(L"D3D_FEATURE_LEVEL_11_0");
		case D3D_FEATURE_LEVEL_11_1:	 return std::wstring_view(L"D3D_FEATURE_LEVEL_11_1");
		case D3D_FEATURE_LEVEL_12_0:	 return std::wstring_view(L"D3D_FEATURE_LEVEL_12_0");
		case D3D_FEATURE_LEVEL_12_1:	 return std::wstring_view(L"D3D_FEATURE_LEVEL_12_1");
		case D3D_FEATURE_LEVEL_12_2:	 return std::wstring_view(L"D3D_FEATURE_LEVEL_12_2");
		default:						 return std::wstring_view(L"UNKNOWN");
		}
	}

	inline constexpr float ScreenToNDCX(uint32_t x, uint32_t width)
	{
		return (2.0f * x) / width - 1.0f;
	}

	inline constexpr float ScreenToNDCY(uint32_t y, uint32_t height)
	{
		return 1.0f - (2.0f * y) / height;
	}
}