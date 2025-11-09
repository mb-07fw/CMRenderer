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

	void CameraComponent::UpdateMatrices() noexcept
	{
		if (ViewDirty)
			CreateViewMatrix();
		if (ProjDirty)
			CreateProjectionMatrix();

		SetClean();
	}

	void CameraComponent::SetOrigin(Float3 origin) noexcept
	{
		Data.Origin = origin;
		ViewDirty = true;
	}

	void CameraComponent::SetLookAtPos(Float3 lookAtPos) noexcept
	{
		Data.LookAtPos = lookAtPos;
		ViewDirty = true;
	}

	void CameraComponent::SetAspect(float aspect) noexcept
	{
		Data.Aspect = aspect;
		ProjDirty = true;
	}

	void CameraComponent::SetFovAngle(float fovAngle) noexcept
	{
		Data.FovAngle = fovAngle;
		ProjDirty = true;
	}

	void CameraComponent::SetNearZ(float nearZ) noexcept
	{
		Data.NearZ = nearZ;
		ProjDirty = true;
	}

	void CameraComponent::SetFarZ(float farZ) noexcept
	{
		Data.FarZ = farZ;
		ProjDirty = true;
	}

	void CameraComponent::SetClean() noexcept
	{
		ViewDirty = false;
		ProjDirty = false;
	}
}