#include "PCH.hpp"
#include "Component.hpp"

namespace CMEngine
{
	void CameraComponent::CreateViewMatrix() noexcept
	{
		Math::ViewMatrixLookAtLH(
			Matrices.View,
			Origin,
			LookAtPos
		);
	}

	void CameraComponent::CreateProjectionMatrix() noexcept
	{
		Math::ProjectionMatrixPerspectiveFovLH(
			Matrices.Proj,
			FovAngle,
			Aspect,
			NearZ,
			FarZ
		);
	}

	void CameraComponent::CreateMatrices() noexcept
	{
		CreateViewMatrix();
		CreateProjectionMatrix();
	}
}