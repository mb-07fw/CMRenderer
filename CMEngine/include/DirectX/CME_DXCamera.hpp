#pragma once

#include <DirectXMath.h>

#include "Core/CME_Camera.hpp"
#include "DirectX/CME_DXUtility.hpp"

namespace CMEngine::DirectXAPI::DX11
{
	#pragma warning(push)
	#pragma warning(disable: 4324)

	class DXCamera
	{
	public:
		DXCamera(const CMCameraData& cameraData, float aspectRatio) noexcept;
		DXCamera() = default;
		~DXCamera() = default;
	public:
		void SetTransform(const CMCommon::CMRigidTransform& rigidTransform) noexcept;
		void SetAspectRatio(float aspectRatio) noexcept;
		void SetAll(const CMCameraData& cameraData, float aspectRatio) noexcept;

		void CalculateViewMatrix() noexcept;
		void CalculateProjectionMatrix() noexcept;
		void CalculateViewProjectionMatrix() noexcept;

		inline [[nodiscard]] float VertFovDegrees() const noexcept { return m_VertFovDegrees; }
		inline [[nodiscard]] float NearZ() const noexcept { return m_NearZ; }
		inline [[nodiscard]] float FarZ() const noexcept { return m_FarZ; }
		inline [[nodiscard]] float AspectRatio() const noexcept { return m_AspectRatio; }

		inline const DirectX::XMMATRIX& ViewMatrix() const noexcept { return m_ViewMatrix; }
		inline const DirectX::XMMATRIX& ProjectionMatrix() const noexcept { return m_ProjectionMatrix; }
		inline const DirectX::XMMATRIX& ViewProjectionMatrix() const noexcept { return m_ViewProjectionMatrix; }
	private:
		float m_VertFovDegrees = 0.0f;
		float m_NearZ = 0.0f;
		float m_FarZ = 0.0f;
		float m_AspectRatio = 0.0f;
		DirectX::XMFLOAT4 m_CameraPos = { 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT3 m_Rotation = { 0.0f, 0.0f, 0.0f };
		const DirectX::XMVECTOR FORWARD_DIRECTION = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // Look down Z axis..
		const DirectX::XMVECTOR UP_DIRECTION = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Y+ is up...
		DirectX::XMMATRIX m_ViewMatrix = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX m_ProjectionMatrix = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX m_ViewProjectionMatrix = DirectX::XMMatrixIdentity();
	};

	#pragma warning(pop)
}