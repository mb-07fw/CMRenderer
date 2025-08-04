#pragma once

#include <DirectXMath.h>

#include "Core/Camera.hpp"
#include "DX/DX11/DX11_Utility.hpp"

namespace CMEngine::DX::DX11
{
#pragma warning(push)
#pragma warning(disable: 4324)

	class Camera
	{
	public:
		Camera(const Core::CameraData& cameraData, float aspectRatio) noexcept;
		Camera() = default;
		~Camera() = default;
	public:
		void SetTransform(const Common::RigidTransform& rigidTransform) noexcept;
		void SetProjection(const Core::CameraData::ProjectionTy& projection) noexcept;
		void SetAspectRatio(float aspectRatio) noexcept;
		void SetAll(const Core::CameraData& cameraData, float aspectRatio) noexcept;

		void CalculateViewMatrix() noexcept;
		void CalculateProjectionMatrix(const Core::CameraData::ProjectionTy& projection) noexcept;
		void CalculateViewProjectionMatrix() noexcept;

		void CalculateOrthographicProjectionMatrix(const Core::OrthographicParams& params) noexcept;
		void CalculatePerspectiveProjectionMatrix(Core::PerspectiveParams params) noexcept;

		inline [[nodiscard]] const Core::OrthographicParams& OrthographicParams() const noexcept { return m_OrthographicParams; }
		inline [[nodiscard]] Core::PerspectiveParams PerspectiveParams() const noexcept { return m_PerspectiveParams; }
		inline [[nodiscard]] float NearZ() const noexcept { return m_NearZ; }
		inline [[nodiscard]] float FarZ() const noexcept { return m_FarZ; }
		inline [[nodiscard]] float AspectRatio() const noexcept { return m_AspectRatio; }

		inline const DirectX::XMMATRIX& ViewMatrix() const noexcept { return m_ViewMatrix; }
		inline const DirectX::XMMATRIX& ProjectionMatrix() const noexcept { return m_ProjectionMatrix; }
		inline const DirectX::XMMATRIX& ViewProjectionMatrix() const noexcept { return m_ViewProjectionMatrix; }
	private:
		Core::OrthographicParams m_OrthographicParams;
		Core::PerspectiveParams m_PerspectiveParams;
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
		Core::ProjectionType m_LastProjectionType = Core::ProjectionType::INVALID;
	};

#pragma warning(pop)
}