#pragma once

#include "Types.hpp"

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

namespace CMEngine::Platform::WinImpl
{
	template <typename Ty>
	using ComPtr = Microsoft::WRL::ComPtr<Ty>;

	inline constexpr CM_ENGINE_API [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float2 float2, float z = 0.0f) noexcept;
	inline constexpr CM_ENGINE_API [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float3 float3) noexcept;

	inline constexpr CM_ENGINE_API [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float2 float2, float z = 0.0f, float w = 1.0f) noexcept;
	inline constexpr CM_ENGINE_API [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float3 float3, float w = 1.0f) noexcept;

	inline constexpr CM_ENGINE_API [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float2 float2, float z) noexcept
	{
		return DirectX::XMFLOAT3(float2.x, float2.y, z);
	}

	inline constexpr CM_ENGINE_API [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float3 float3) noexcept
	{
		return DirectX::XMFLOAT3(float3.x, float3.y, float3.z);
	}

	inline constexpr CM_ENGINE_API [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float2 float2, float z, float w) noexcept
	{
		return DirectX::XMFLOAT4(float2.x, float2.y, z, w);
	}

	inline constexpr CM_ENGINE_API [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float3 float3, float w) noexcept
	{
		return DirectX::XMFLOAT4(float3.x, float3.y, float3.z, w);

	}
}