#include "PCH.hpp"
#include "Component.hpp"

namespace CMEngine
{
	void CameraComponent::CreateViewMatrix() noexcept
	{
		Math::ViewMatrixLookAtLH(
			Matrices.View,
			Data.Origin,
			Data.LookAtPos
		);
	}

	void CameraComponent::CreateProjectionMatrix() noexcept
	{
		Math::ProjectionMatrixPerspectiveFovLH(
			Matrices.Proj,
			Data.FovAngle,
			Data.Aspect,
			Data.NearZ,
			Data.FarZ
		);
	}

	void CameraComponent::CreateMatrices() noexcept
	{
		CreateViewMatrix();
		CreateProjectionMatrix();
	}
}