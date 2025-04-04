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
		inline const DirectX::XMMATRIX& ViewProjectionMatrix() const noexcept { return m_ViewProjectionMatrix; }
	private:
		DirectX::XMMATRIX m_ViewMatrix = {};
		DirectX::XMMATRIX m_ProjectionMatrix = {};
		DirectX::XMMATRIX m_ViewProjectionMatrix = {};
	};
}