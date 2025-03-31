#pragma once

#include <d3dcommon.h>
#include <string_view>

namespace CMRenderer::CMDirectX::Utility
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
}