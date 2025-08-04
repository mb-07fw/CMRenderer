#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include <string_view>
#include <cstdint>

#include "Common/Types.hpp"

namespace CMEngine::DX::Utility
{
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

	inline constexpr float ScreenToNDCX(uint32_t x, uint32_t width) noexcept
	{
		return (2.0f * x) / width - 1.0f;
	}

	inline constexpr float ScreenToNDCY(uint32_t y, uint32_t height) noexcept
	{
		return 1.0f - (2.0f * y) / height;
	}

	inline constexpr DirectX::XMFLOAT4 ToXMFloat4(Common::Float3 float3, float w = 1.0f) noexcept
	{
		return DirectX::XMFLOAT4(float3.x, float3.y, float3.z, w);
	}

	inline constexpr DirectX::XMFLOAT3 ToXMFloat3(Common::Float3 float3) noexcept
	{
		return DirectX::XMFLOAT3(float3.x, float3.y, float3.z);
	}
}