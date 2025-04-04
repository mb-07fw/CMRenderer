#include "Core/CMPCH.hpp"
#include "Internal/DirectX/DXCamera.hpp"

namespace CMRenderer
{
	DXCamera::DXCamera(float worldPosX, float worldPosY, float worldPosZ, float vFovDegrees, float aspectRatio) noexcept
	{
		DirectX::XMVECTOR cameraWorldPos = DirectX::XMVectorSet(worldPosX, worldPosY, worldPosZ, 0.0f);
		DirectX::XMVECTOR cameraOrientation = DirectX::XMVectorZero(); // Look at origin...
		DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Y+ is up...

		m_ViewMatrix = DirectX::XMMatrixLookAtLH(
			cameraWorldPos, cameraOrientation, upDirection
		);

		m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XMConvertToRadians(vFovDegrees), // 45 degree fov (in radians)
			aspectRatio,
			0.5f, // Near plane Z.
			100.0f // Far plane Z.
		);

		m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
	}
}