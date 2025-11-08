#pragma once

#include "Types.hpp"

#include <DirectXMath.h>

namespace CMEngine::Math
{
	using Mat4 = DirectX::XMMATRIX;

	inline constexpr [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float2 float2, float z = 0.0f) noexcept;
	inline constexpr [[nodiscard]] DirectX::XMFLOAT3 ToXMFloat3(Float3 float3) noexcept;

	inline constexpr [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float2 float2, float z = 0.0f, float w = 1.0f) noexcept;
	inline constexpr [[nodiscard]] DirectX::XMFLOAT4 ToXMFloat4(Float3 float3, float w = 1.0f) noexcept;

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

	inline [[nodiscard]] Mat4 IdentityMatrix() noexcept
	{
		return DirectX::XMMatrixIdentity();
	}

	inline void ViewMatrixLookAtLH(
		Mat4& outMatrix,
		const Float3& origin,
		const Float3& lookAtPos
	) noexcept
	{
		DirectX::XMFLOAT3 cameraPos = ToXMFloat3(origin);
		DirectX::XMFLOAT3 cameraFocusPos = ToXMFloat3(lookAtPos);
		DirectX::XMFLOAT3 upDirection = { 0.0f, 1.0f, 0.0f };

		DirectX::XMVECTOR cameraPosVec = DirectX::XMLoadFloat3(&cameraPos);
		DirectX::XMVECTOR cameraFocusPosVec = DirectX::XMLoadFloat3(&cameraFocusPos);
		DirectX::XMVECTOR upDirectionVec = DirectX::XMLoadFloat3(&upDirection);

		outMatrix = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixLookAtLH(
				cameraPosVec,
				cameraFocusPosVec,
				upDirectionVec
			)
		);
	}

	inline void ProjectionMatrixPerspectiveFovLH(
		Mat4& outMatrix,
		float fovAngle,
		float aspectRatio,
		float nearZ,
		float farZ
	)
	{
		float fovRad = AngleToRadians(fovAngle);

		outMatrix = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixPerspectiveFovLH(
				fovRad,
				aspectRatio,
				nearZ,
				farZ
			)
		);
	}

	inline void TransformMatrix(Mat4& outMatrix, const Transform& transform) noexcept
	{
		outMatrix = DirectX::XMMatrixIdentity();
		
		if (!transform.Scaling.IsNearEqual(1.0f))
			outMatrix *= DirectX::XMMatrixScaling(
				transform.Scaling.x,
				transform.Scaling.y,
				transform.Scaling.z
			);
		if (!transform.Rotation.IsNearEqual(0.0f))
			outMatrix *= DirectX::XMMatrixRotationRollPitchYaw(
				AngleToRadians(transform.Rotation.x),
				AngleToRadians(transform.Rotation.y),
				AngleToRadians(transform.Rotation.z)
			);
		if (!transform.Translation.IsNearEqual(0.0f))
			outMatrix *= DirectX::XMMatrixTranslation(
				transform.Translation.x,
				transform.Translation.y,
				transform.Translation.z
			);

		outMatrix = DirectX::XMMatrixTranspose(outMatrix);
	}
}