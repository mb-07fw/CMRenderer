#include "Core/CME_PCH.hpp"
#include "DirectX/CME_DXCamera.hpp"
#include "CMC_Macros.hpp"

namespace CMEngine::DirectXAPI::DX11
{
	DXCamera::DXCamera(const CMCameraData& cameraData, float aspectRatio) noexcept
	{
		SetAll(cameraData, aspectRatio);
	}

	void DXCamera::SetAll(const CMCameraData& cameraData, float aspectRatio) noexcept
	{
		CalculateViewMatrix(cameraData.RigidTransform);
		CalculateProjectionMatrix(cameraData.Projection, aspectRatio, cameraData.NearZ, cameraData.FarZ);
		CalculateViewProjectionMatrix();
	}

	void DXCamera::SetTransform(const CMCommon::CMRigidTransform& rigidTransform) noexcept
	{
		CalculateViewMatrix(rigidTransform);
		CalculateViewProjectionMatrix();
	}

	void DXCamera::SetAspectRatio(float aspectRatio) noexcept
	{
		m_AspectRatio = aspectRatio;

		CalculateProjectionMatrix(m_Projection, aspectRatio, m_NearZ, m_FarZ);
		CalculateViewProjectionMatrix();
	}

	void DXCamera::CalculateProjectionMatrix(
		const VariantProj& projection,
		float aspectRatio,
		float nearZ,
		float farZ
	) noexcept
	{
		std::visit([&](auto&& proj)
			{
				// Get underlying type.. (remove const or reference qualifiers from type)
				using Ty = std::decay_t<decltype(proj)>;

				if constexpr (std::is_same_v<Ty, CMOrthographicParams>)
					CalculateOrthographicProjectionMatrix(
						proj,
						aspectRatio,
						nearZ,
						farZ
					);
				else if constexpr (std::is_same_v<Ty, CMPerspectiveParams>)
					CalculatePerspectiveProjectionMatrix(
						proj,
						aspectRatio,
						nearZ,
						farZ
					);

			}, projection);
	}

	void DXCamera::CalculateViewMatrix(const CMCommon::CMRigidTransform& rigidTransform) noexcept
	{
		m_CameraPos = Utility::ToXMFloat4(rigidTransform.Translation, 1.0f);
		m_Rotation = Utility::ToXMFloat3(rigidTransform.Rotation);

		DirectX::XMVECTOR posV = DirectX::XMLoadFloat4(&m_CameraPos);

		/* X = Pitch, Y = Yaw, Z = Roll */
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
			m_Rotation.x,
			m_Rotation.y,
			m_Rotation.z
		);

		/* Apply the rotation matrix to transform the forward vector
		 *   into world space — where the camera is actually looking. */
		DirectX::XMVECTOR lookDirection = DirectX::XMVector3TransformNormal(
			FORWARD_DIRECTION,
			rotationMatrix
		);

		/* The camera’s target position (the "look-at" point) is
		 *   the camera's position plus its forward direction. */
		DirectX::XMVECTOR targetPosV = DirectX::XMVectorAdd(posV, lookDirection);

		m_ViewMatrix = DirectX::XMMatrixLookAtLH(posV, targetPosV, UP_DIRECTION);
	}

	void DXCamera::CalculateOrthographicProjectionMatrix(
		const CMOrthographicParams& orthographic,
		float aspectRatio,
		float nearZ,
		float farZ
	) noexcept
	{
		CMOrthographicParams newOrtho = orthographic;

		/* Transform x and y boundaries from [0, x] to [-x / 2, x / 2].
		 * Ex. [0, 4] -> [-2, 2] */
		newOrtho.View.Left = -newOrtho.View.Right / 2;
		newOrtho.View.Right = newOrtho.View.Right / 2;
		newOrtho.View.Top = -newOrtho.View.Bottom / 2;
		newOrtho.View.Bottom = newOrtho.View.Bottom / 2;

		m_Projection = newOrtho;
		m_AspectRatio = aspectRatio;
		m_NearZ = nearZ;
		m_FarZ = farZ;

		m_ProjectionMatrix = DirectX::XMMatrixOrthographicOffCenterLH(
			newOrtho.View.Left,
			newOrtho.View.Right,
			newOrtho.View.Bottom,
			newOrtho.View.Top,
			m_NearZ,
			m_FarZ
		);

		m_LastProjectionType = CMProjectionType::ORTHOGRAPHIC;
	}

	void DXCamera::CalculatePerspectiveProjectionMatrix(
		const CMPerspectiveParams& perspective,
		float aspectRatio,
		float nearZ,
		float farZ
	) noexcept
	{
		m_Projection = perspective;
		m_AspectRatio = aspectRatio;
		m_NearZ = nearZ;
		m_FarZ = farZ;

		m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
			perspective.VertFovDegrees,
			m_AspectRatio,
			m_NearZ,
			m_FarZ
		);

		m_LastProjectionType = CMProjectionType::PERSPECTIVE;
	}

	void DXCamera::CalculateViewProjectionMatrix() noexcept
	{
		m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
	}
}