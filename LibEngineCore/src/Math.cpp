#include "PCH.hpp"
#include "Math.hpp"

namespace CMEngine::Math
{
	[[nodiscard]] float Length(Vec2 v) noexcept
	{
		/* sqrt(x^2 + y^2)... */
		return std::sqrt(v.x * v.x + v.y * v.y);
	}

	[[nodiscard]] float Length(Vec3 v) noexcept
	{
		/* sqrt(x^2 + y^2 + z^2)... */
		return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	[[nodiscard]] Vec2 Normalize(Vec2 v) noexcept
	{
		float lenSq = Length(v);

		if (IsNearEqualFloat(lenSq, 0.0f))
			return Vec2(0.0f, 0.0f);

		float invLen = 1.0f / std::sqrt(lenSq);
		return v * invLen;
	}

	[[nodiscard]] Vec3 Normalize(Vec3 v) noexcept
	{
		float lenSq = Length(v);

		if (IsNearEqualFloat(lenSq, 0.0f))
			return Vec3(0.0f, 0.0f);

		float invLen = 1.0f / std::sqrt(lenSq);
		return v * invLen;
	}

	[[nodiscard]] Mat4 IdentityMatrix() noexcept
	{
		return DirectX::XMMatrixIdentity();
	}

	void ViewMatrixLookAtLH(
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

	void ProjectionMatrixPerspectiveFovLH(
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

	void TransformMatrix(Mat4& outMatrix, const Transform& transform) noexcept
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