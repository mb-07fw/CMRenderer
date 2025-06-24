#include "Core/CME_PCH.hpp"
#include "DirectX/CME_DXCamera.hpp"

namespace CMEngine::DirectXAPI::DX11
{
	DXCamera::DXCamera(const CMCameraData& cameraData, float aspectRatio) noexcept
	{	
		SetAll(cameraData, aspectRatio);
	}

	void DXCamera::SetTransform(const CMCommon::CMRigidTransform& rigidTransform) noexcept
	{
		m_CameraPos = Utility::ToXMFloat4(rigidTransform.Translation, 1.0f);
		m_Rotation = Utility::ToXMFloat3(rigidTransform.Rotation);

		CalculateViewMatrix();
		CalculateViewProjectionMatrix();
	}

	void DXCamera::SetAspectRatio(float aspectRatio) noexcept
	{
		m_AspectRatio = aspectRatio;

		CalculateProjectionMatrix();
		CalculateViewProjectionMatrix();
	}

	void DXCamera::SetAll(const CMCameraData& cameraDataRef, float aspectRatio) noexcept
	{
		m_CameraPos = Utility::ToXMFloat4(cameraDataRef.RigidTransform.Translation, 1.0f);
		m_Rotation = Utility::ToXMFloat3(cameraDataRef.RigidTransform.Rotation);

		m_VertFovDegrees = cameraDataRef.VertFovDegrees;

		m_NearZ = cameraDataRef.NearZ;
		m_FarZ = cameraDataRef.FarZ;

		m_AspectRatio = aspectRatio;

		CalculateViewMatrix();
		CalculateProjectionMatrix();
		CalculateViewProjectionMatrix();
	}

	void DXCamera::CalculateViewMatrix() noexcept
	{
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat4(&m_CameraPos);

		/* X -> Pitch, Y -> Yaw, Z -> Roll */
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		DirectX::XMVECTOR lookDirection = DirectX::XMVector3TransformNormal(FORWARD_DIRECTION, rotationMatrix);

		DirectX::XMVECTOR targetFocusPos = DirectX::XMVectorAdd(pos, lookDirection);

		m_ViewMatrix = DirectX::XMMatrixLookAtLH(pos, targetFocusPos, UP_DIRECTION);
	}

	void DXCamera::CalculateProjectionMatrix() noexcept
	{
		m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_VertFovDegrees, m_AspectRatio, m_NearZ, m_FarZ);
	}

	void DXCamera::CalculateViewProjectionMatrix() noexcept
	{
		m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
	}
}