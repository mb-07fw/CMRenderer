#pragma once

#include <DirectXMath.h>

namespace CMRenderer
{
	class DXCamera
	{
	public:
		DXCamera(float worldPosX, float worldPosY, float worldPosZ, float vFovDegrees, float aspectRatio) noexcept;
		~DXCamera() = default;
	public:
		void TranslatePos(float offsetX, float offsetY, float offsetZ) noexcept;

		void CalculateViewMatrix() noexcept;
		void CalculateViewProjectionMatrix() noexcept;

		inline const DirectX::XMMATRIX& ViewMatrix() const noexcept { return m_ViewMatrix; }
		inline const DirectX::XMMATRIX& ProjectionMatrix() const noexcept { return m_ProjectionMatrix; }
		inline const DirectX::XMMATRIX& ViewProjectionMatrix() const noexcept { return m_ViewProjectionMatrix; }
	private:
		DirectX::XMFLOAT4 m_CameraPos = { 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMVECTOR m_CameraOrientation = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // Look at origin...
		DirectX::XMVECTOR m_UpDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Y+ is up...
		DirectX::XMMATRIX m_ViewMatrix = {};
		DirectX::XMMATRIX m_ProjectionMatrix = {};
		DirectX::XMMATRIX m_ViewProjectionMatrix = {};
	};
}