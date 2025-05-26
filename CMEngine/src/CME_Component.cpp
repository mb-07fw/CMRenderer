#include "CME_PCH.hpp"
#include "CME_Component.hpp"

namespace CMEngine
{
	CMTransformComponent::CMTransformComponent(CMCommon::CMTransform transform) noexcept
		: Transform(transform)
	{
	}

	[[nodiscard]] bool CMTransformComponent::operator==(const CMTransformComponent& otherRef) noexcept
	{
		return IsEqual(otherRef);
	}

	[[nodiscard]] bool CMTransformComponent::operator==(const CMCommon::CMTransform& transformRef) noexcept
	{
		return IsEqual(transformRef);
	}

	[[nodiscard]] bool CMTransformComponent::IsEqual(const CMTransformComponent& otherRef) noexcept
	{
		return Transform == otherRef.Transform;
	}

	[[nodiscard]] bool CMTransformComponent::IsEqual(const CMCommon::CMTransform& transformRef) noexcept
	{
		return Transform == transformRef;
	}

	[[nodiscard]] bool CMTransformComponent::IsNearEqual(const CMTransformComponent& otherRef, float epsilon) noexcept
	{
		return Transform.IsNearEqual(otherRef.Transform, epsilon);
	}

	[[nodiscard]] bool CMTransformComponent::IsNearEqual(const CMCommon::CMTransform& transformRef, float epsilon) noexcept
	{
		return Transform.IsNearEqual(transformRef, epsilon);
	}

	CMRigidTransformComponent::CMRigidTransformComponent(CMCommon::CMRigidTransform rigidTransform) noexcept
		: RigidTransform(rigidTransform)
	{
	}

	[[nodiscard]] bool CMRigidTransformComponent::operator==(const CMRigidTransformComponent& otherRef) noexcept
	{
		return IsEqual(otherRef);
	}

	[[nodiscard]] bool CMRigidTransformComponent::operator==(const CMCommon::CMRigidTransform& rigidTransformRef) noexcept
	{
		return IsEqual(rigidTransformRef);
	}

	[[nodiscard]] bool CMRigidTransformComponent::IsEqual(const CMRigidTransformComponent& otherRef) noexcept
	{
		return RigidTransform == otherRef.RigidTransform;
	}

	[[nodiscard]] bool CMRigidTransformComponent::IsEqual(const CMCommon::CMRigidTransform& rigidTransformRef) noexcept
	{
		return RigidTransform == rigidTransformRef;
	}

	[[nodiscard]] bool CMRigidTransformComponent::IsNearEqual(const CMRigidTransformComponent& otherRef, float epsilon) noexcept
	{
		return RigidTransform.IsNearEqual(otherRef.RigidTransform, epsilon);
	}

	[[nodiscard]] bool CMRigidTransformComponent::IsNearEqual(const CMCommon::CMRigidTransform& rigidTransformRef, float epsilon) noexcept
	{
		return RigidTransform.IsNearEqual(rigidTransformRef, epsilon);
	}
}