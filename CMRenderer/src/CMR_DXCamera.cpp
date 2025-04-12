#include "CMR_PCH.hpp"
#include "CMR_DXCamera.hpp"

namespace CMRenderer
{
	DXCamera::DXCamera(float worldPosX, float worldPosY, float worldPosZ, float vFovDegrees, float aspectRatio) noexcept
	{
		m_CameraPos = DirectX::XMFLOAT4(worldPosX, worldPosY, worldPosZ, 1.0f);
			
		CalculateViewMatrix();

		m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XMConvertToRadians(vFovDegrees), // 45 degree fov (in radians)
			aspectRatio,
			0.5f, // Near plane Z.
			100.0f // Far plane Z.
		);

		CalculateViewProjectionMatrix();
	}

	void DXCamera::TranslatePos(float offsetX, float offsetY, float offsetZ) noexcept
	{
		m_CameraPos.x += offsetX;
		m_CameraPos.y += offsetY;
		m_CameraPos.z += offsetZ;

		CalculateViewMatrix();
		CalculateViewProjectionMatrix();
	}

	void DXCamera::CalculateViewMatrix() noexcept
	{
		DirectX::XMVECTOR vecPos = DirectX::XMLoadFloat4(&m_CameraPos);

		m_ViewMatrix = DirectX::XMMatrixLookAtLH(
			vecPos, m_CameraOrientation, m_UpDirection
		);
	}

	void DXCamera::CalculateViewProjectionMatrix() noexcept
	{
		m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
	}
}