#include "Core/PCH.hpp"
#include "Core/Component.hpp"

namespace CMEngine::Core
{
	TransformComponent::TransformComponent(Common::Transform transform) noexcept
		: Transform(transform)
	{
	}

	[[nodiscard]] bool TransformComponent::operator==(const TransformComponent& other) noexcept
	{
		return IsEqual(other);
	}

	[[nodiscard]] bool TransformComponent::operator==(const Common::Transform& transform) noexcept
	{
		return IsEqual(transform);
	}

	[[nodiscard]] bool TransformComponent::IsEqual(const TransformComponent& other) noexcept
	{
		return Transform == other.Transform;
	}

	[[nodiscard]] bool TransformComponent::IsEqual(const Common::Transform& transform) noexcept
	{
		return Transform == transform;
	}

	[[nodiscard]] bool TransformComponent::IsNearEqual(const TransformComponent& other, float epsilon) noexcept
	{
		return Transform.IsNearEqual(other.Transform, epsilon);
	}

	[[nodiscard]] bool TransformComponent::IsNearEqual(const Common::Transform& transform, float epsilon) noexcept
	{
		return Transform.IsNearEqual(transform, epsilon);
	}

	RigidTransformComponent::RigidTransformComponent(Common::RigidTransform rigidTransform) noexcept
		: RigidTransform(rigidTransform)
	{
	}

	[[nodiscard]] bool RigidTransformComponent::operator==(const RigidTransformComponent& other) noexcept
	{
		return IsEqual(other);
	}

	[[nodiscard]] bool RigidTransformComponent::operator==(const Common::RigidTransform& rigidTransform) noexcept
	{
		return IsEqual(rigidTransform);
	}

	[[nodiscard]] bool RigidTransformComponent::IsEqual(const RigidTransformComponent& other) noexcept
	{
		return RigidTransform == other.RigidTransform;
	}

	[[nodiscard]] bool RigidTransformComponent::IsEqual(const Common::RigidTransform& rigidTransform) noexcept
	{
		return RigidTransform == rigidTransform;
	}

	[[nodiscard]] bool RigidTransformComponent::IsNearEqual(const RigidTransformComponent& other, float epsilon) noexcept
	{
		return RigidTransform.IsNearEqual(other.RigidTransform, epsilon);
	}

	[[nodiscard]] bool RigidTransformComponent::IsNearEqual(const Common::RigidTransform& rigidTransform, float epsilon) noexcept
	{
		return RigidTransform.IsNearEqual(rigidTransform, epsilon);
	}

	MeshComponent::MeshComponent(
		Common::Transform transform,
		Asset::AssetHandle handle,
		DX::DX11::ShaderSetType setType
	) noexcept
		: Transform(transform),
		  Handle(handle),
		  SetType(setType)
	{
	}
}