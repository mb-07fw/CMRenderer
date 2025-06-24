#include "Core/CME_PCH.hpp"
#include "Core/CME_Component.hpp"

namespace CMEngine
{
	CMTransformComponent::CMTransformComponent(CMCommon::CMTransform transform) noexcept
		: Transform(transform)
	{
	}

	[[nodiscard]] bool CMTransformComponent::operator==(const CMTransformComponent& other) noexcept
	{
		return IsEqual(other);
	}

	[[nodiscard]] bool CMTransformComponent::operator==(const CMCommon::CMTransform& transform) noexcept
	{
		return IsEqual(transform);
	}

	[[nodiscard]] bool CMTransformComponent::IsEqual(const CMTransformComponent& other) noexcept
	{
		return Transform == other.Transform;
	}

	[[nodiscard]] bool CMTransformComponent::IsEqual(const CMCommon::CMTransform& transform) noexcept
	{
		return Transform == transform;
	}

	[[nodiscard]] bool CMTransformComponent::IsNearEqual(const CMTransformComponent& other, float epsilon) noexcept
	{
		return Transform.IsNearEqual(other.Transform, epsilon);
	}

	[[nodiscard]] bool CMTransformComponent::IsNearEqual(const CMCommon::CMTransform& transform, float epsilon) noexcept
	{
		return Transform.IsNearEqual(transform, epsilon);
	}

	CMRigidTransformComponent::CMRigidTransformComponent(CMCommon::CMRigidTransform rigidTransform) noexcept
		: RigidTransform(rigidTransform)
	{
	}

	[[nodiscard]] bool CMRigidTransformComponent::operator==(const CMRigidTransformComponent& other) noexcept
	{
		return IsEqual(other);
	}

	[[nodiscard]] bool CMRigidTransformComponent::operator==(const CMCommon::CMRigidTransform& rigidTransform) noexcept
	{
		return IsEqual(rigidTransform);
	}

	[[nodiscard]] bool CMRigidTransformComponent::IsEqual(const CMRigidTransformComponent& other) noexcept
	{
		return RigidTransform == other.RigidTransform;
	}

	[[nodiscard]] bool CMRigidTransformComponent::IsEqual(const CMCommon::CMRigidTransform& rigidTransform) noexcept
	{
		return RigidTransform == rigidTransform;
	}

	[[nodiscard]] bool CMRigidTransformComponent::IsNearEqual(const CMRigidTransformComponent& other, float epsilon) noexcept
	{
		return RigidTransform.IsNearEqual(other.RigidTransform, epsilon);
	}

	[[nodiscard]] bool CMRigidTransformComponent::IsNearEqual(const CMCommon::CMRigidTransform& rigidTransform, float epsilon) noexcept
	{
		return RigidTransform.IsNearEqual(rigidTransform, epsilon);
	}

	CMMeshComponent::CMMeshComponent(
		CMCommon::CMTransform transform,
		Asset::CMAssetHandle handle,
		DirectXAPI::DX11::DXShaderSetType setType
	) noexcept
		: Transform(transform),
		  Handle(handle),
		  SetType(setType)
	{
	}
}