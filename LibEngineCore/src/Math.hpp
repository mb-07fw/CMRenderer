#pragma once

#include "Types.hpp"

#include <DirectXMath.h>

namespace CMEngine::Math
{
	using Mat4 = DirectX::XMMATRIX;

	inline constexpr [[nodiscard]] float AngleToRadians(float angle) noexcept;

	inline constexpr [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float2 float2, float z = 0.0f) noexcept;
	inline constexpr [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float3 float3) noexcept;

	inline constexpr [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float2 float2, float z = 0.0f, float w = 1.0f) noexcept;
	inline constexpr [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float3 float3, float w = 1.0f) noexcept;

	/* Returns the length (magnitude) of the provided vector. */
	[[nodiscard]] float Length(Vec2 v) noexcept;
	[[nodiscard]] float Length(Vec3 v) noexcept;

	/* Normalizes the length (magnitude) of the provided vector to unit length. (1)*/
	[[nodiscard]] Vec2 Normalize(Vec2 v) noexcept;
	[[nodiscard]] Vec3 Normalize(Vec3 v) noexcept;

	[[nodiscard]] Mat4 IdentityMatrix() noexcept;

	void ViewMatrixLookAtLH(
		Mat4& outMatrix,
		const Float3& origin,
		const Float3& lookAtPos
	) noexcept;

	void ProjectionMatrixPerspectiveFovLH(
		Mat4& outMatrix,
		float fovAngle,
		float aspectRatio,
		float nearZ,
		float farZ
	);

	void TransformMatrix(
		Mat4& outMatrix,
		const Transform& transform
	) noexcept;

	inline constexpr [[nodiscard]] float AngleToRadians(float angle) noexcept
	{
		return DirectX::XMConvertToRadians(angle);
	}

	inline constexpr [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float2 float2, float z) noexcept
	{
		return DirectX::XMFLOAT3(float2.x, float2.y, z);
	}

	inline constexpr [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float3 float3) noexcept
	{
		return DirectX::XMFLOAT3(float3.x, float3.y, float3.z);
	}

	inline constexpr [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float2 float2, float z, float w) noexcept
	{
		return DirectX::XMFLOAT4(float2.x, float2.y, z, w);
	}

	inline constexpr [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float3 float3, float w) noexcept
	{
		return DirectX::XMFLOAT4(float3.x, float3.y, float3.z, w);
	}
}