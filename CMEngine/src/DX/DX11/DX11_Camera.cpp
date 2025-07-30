#include "Core/PCH.hpp"
#include "DX/DX11/DX11_Camera.hpp"

namespace CMEngine::DX::DX11
{
	Camera::Camera(const Core::CameraData& cameraData, float aspectRatio) noexcept
	{
		SetAll(cameraData, aspectRatio);
	}

	void Camera::SetTransform(const Common::RigidTransform& rigidTransform) noexcept
	{
		m_CameraPos = Utility::ToXMFloat4(rigidTransform.Translation, 1.0f);
		m_Rotation = Utility::ToXMFloat3(rigidTransform.Rotation);

		CalculateViewMatrix();
		CalculateViewProjectionMatrix();
	}

	void Camera::SetProjection(const Core::CameraData::ProjectionTy& projection) noexcept
	{
		CalculateProjectionMatrix(projection);
		CalculateViewProjectionMatrix();
	}

	void Camera::SetAspectRatio(float aspectRatio) noexcept
	{
		m_AspectRatio = aspectRatio;

		switch (m_LastProjectionType)
		{
		case Core::ProjectionType::ORTHOGRAPHIC:
			CalculateOrthographicProjectionMatrix(m_OrthographicParams);
			break;
		case Core::ProjectionType::PERSPECTIVE:
			CalculatePerspectiveProjectionMatrix(m_PerspectiveParams);
			break;
		case Core::ProjectionType::INVALID: [[fallthrough]];
		default:
			return;
		}

		CalculateViewProjectionMatrix();
	}

	void Camera::SetAll(const Core::CameraData& cameraDataRef, float aspectRatio) noexcept
	{
		m_CameraPos = Utility::ToXMFloat4(cameraDataRef.RigidTransform.Translation, 1.0f);
		m_Rotation = Utility::ToXMFloat3(cameraDataRef.RigidTransform.Rotation);

		m_NearZ = cameraDataRef.NearZ;
		m_FarZ = cameraDataRef.FarZ;

		m_AspectRatio = aspectRatio;

		CalculateViewMatrix();
		CalculateProjectionMatrix(cameraDataRef.Projection);
		CalculateViewProjectionMatrix();
	}

	void Camera::CalculateViewMatrix() noexcept
	{
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat4(&m_CameraPos);

		/* X -> Pitch, Y -> Yaw, Z -> Roll */
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		DirectX::XMVECTOR lookDirection = DirectX::XMVector3TransformNormal(FORWARD_DIRECTION, rotationMatrix);

		DirectX::XMVECTOR targetFocusPos = DirectX::XMVectorAdd(pos, lookDirection);

		m_ViewMatrix = DirectX::XMMatrixLookAtLH(pos, targetFocusPos, UP_DIRECTION);
	}

	void Camera::CalculateProjectionMatrix(const Core::CameraData::ProjectionTy& projection) noexcept
	{
		std::visit([&](auto&& proj)
			{
				using Ty = std::decay_t<decltype(proj)>;

				if constexpr (std::is_same_v<Ty, Core::OrthographicParams>)
					CalculateOrthographicProjectionMatrix(proj);
				else if constexpr (std::is_same_v<Ty, Core::PerspectiveParams>)
					CalculatePerspectiveProjectionMatrix(proj);

			}, projection);
	}

	void Camera::CalculateViewProjectionMatrix() noexcept
	{
		m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
	}

	void Camera::CalculateOrthographicProjectionMatrix(const Core::OrthographicParams& params) noexcept
	{
		m_OrthographicParams = params;

		m_ProjectionMatrix = DirectX::XMMatrixOrthographicOffCenterLH(
			params.View.Left,
			params.View.Right,
			params.View.Bottom,
			params.View.Top,
			m_NearZ,
			m_FarZ
		);

		m_LastProjectionType = Core::ProjectionType::ORTHOGRAPHIC;
	}

	void Camera::CalculatePerspectiveProjectionMatrix(Core::PerspectiveParams params) noexcept
	{
		m_PerspectiveParams = params;

		m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(params.VertFovDegrees, m_AspectRatio, m_NearZ, m_FarZ);

		m_LastProjectionType = Core::ProjectionType::PERSPECTIVE;
	}
}