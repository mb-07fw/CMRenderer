#pragma once

#include <vector>
#include <cstdint>

#include "Core/AssetHandle.hpp"
#include "DX/DX11/DX11_ShaderData.hpp"
#include "Common/Types.hpp"

namespace CMEngine::Core
{
	struct Component 
	{
		Component() = default;
		~Component() = default;
	};

	struct TransformComponent : public Component
	{
		TransformComponent(Common::Transform transform = Common::Transform()) noexcept;
		~TransformComponent() = default;

		[[nodiscard]] bool operator==(const TransformComponent& other) noexcept;
		[[nodiscard]] bool operator==(const Common::Transform& transform) noexcept;
		[[nodiscard]] bool IsEqual(const TransformComponent& other) noexcept;
		[[nodiscard]] bool IsEqual(const Common::Transform& transform) noexcept;
		[[nodiscard]] bool IsNearEqual(const TransformComponent& other, float epsilon = Common::G_NEAR_EQUAL_FLOAT_EPSILON) noexcept;
		[[nodiscard]] bool IsNearEqual(const Common::Transform& transform, float epsilon = Common::G_NEAR_EQUAL_FLOAT_EPSILON) noexcept;

		Common::Transform Transform;
	};

	struct RigidTransformComponent : public Component
	{
		RigidTransformComponent(Common::RigidTransform rigidTransform = Common::RigidTransform()) noexcept;
		~RigidTransformComponent() = default;

		[[nodiscard]] bool operator==(const RigidTransformComponent& other) noexcept;
		[[nodiscard]] bool operator==(const Common::RigidTransform& rigidTransform) noexcept;
		[[nodiscard]] bool IsEqual(const RigidTransformComponent& other) noexcept;
		[[nodiscard]] bool IsEqual(const Common::RigidTransform& rigidTransform) noexcept;
		[[nodiscard]] bool IsNearEqual(const RigidTransformComponent& other, float epsilon = Common::G_NEAR_EQUAL_FLOAT_EPSILON) noexcept;
		[[nodiscard]] bool IsNearEqual(const Common::RigidTransform& rigidTransform, float epsilon = Common::G_NEAR_EQUAL_FLOAT_EPSILON) noexcept;

		Common::RigidTransform RigidTransform;
	};

	struct MeshComponent : public Component
	{
		MeshComponent(
			Common::Transform transform,
			Asset::AssetHandle handle,
			DX::DX11::ShaderSetType setType
		) noexcept;

		MeshComponent() = default;
		~MeshComponent() = default;

		Common::Transform Transform = {};
		Asset::AssetHandle Handle = Asset::AssetHandle::Invalid();
		DX::DX11::ShaderSetType SetType = DX::DX11::ShaderSetType::INVALID;
	};

	using CameraComponent = RigidTransformComponent;
}