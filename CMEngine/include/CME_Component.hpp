#pragma once

#include "CME_AssetHandle.hpp"
#include "CMR_DXShaderData.hpp"
#include "CMC_Types.hpp"

#include <vector>
#include <cstdint>

namespace CMEngine
{
	struct CMComponent 
	{
		CMComponent() = default;
		~CMComponent() = default;
	};

	struct CMTransformComponent : public CMComponent
	{
		CMTransformComponent(CMCommon::CMTransform transform = CMCommon::CMTransform()) noexcept;
		~CMTransformComponent() = default;

		[[nodiscard]] bool operator==(const CMTransformComponent& otherRef) noexcept;
		[[nodiscard]] bool operator==(const CMCommon::CMTransform& transformRef) noexcept;
		[[nodiscard]] bool IsEqual(const CMTransformComponent& otherRef) noexcept;
		[[nodiscard]] bool IsEqual(const CMCommon::CMTransform& transformRef) noexcept;
		[[nodiscard]] bool IsNearEqual(const CMTransformComponent& otherRef, float epsilon = CMCommon::G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept;
		[[nodiscard]] bool IsNearEqual(const CMCommon::CMTransform& transformRef, float epsilon = CMCommon::G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept;

		CMCommon::CMTransform Transform;
	};

	struct CMRigidTransformComponent : public CMComponent
	{
		CMRigidTransformComponent(CMCommon::CMRigidTransform rigidTransform = CMCommon::CMRigidTransform()) noexcept;
		~CMRigidTransformComponent() = default;

		[[nodiscard]] bool operator==(const CMRigidTransformComponent& otherRef) noexcept;
		[[nodiscard]] bool operator==(const CMCommon::CMRigidTransform& rigidTransformRef) noexcept;
		[[nodiscard]] bool IsEqual(const CMRigidTransformComponent& otherRef) noexcept;
		[[nodiscard]] bool IsEqual(const CMCommon::CMRigidTransform& rigidTransformRef) noexcept;
		[[nodiscard]] bool IsNearEqual(const CMRigidTransformComponent& otherRef, float epsilon = CMCommon::G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept;
		[[nodiscard]] bool IsNearEqual(const CMCommon::CMRigidTransform& rigidTransformRef, float epsilon = CMCommon::G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept;

		CMCommon::CMRigidTransform RigidTransform;
	};

	struct CMMeshComponent : public CMComponent
	{
		CMMeshComponent(
			CMCommon::CMTransform transform,
			Asset::CMAssetHandle handle,
			CMRenderer::CMDirectX::DXShaderSetType setType
		) noexcept;

		CMMeshComponent() = default;
		~CMMeshComponent() = default;

		CMCommon::CMTransform Transform = {};
		Asset::CMAssetHandle Handle = Asset::CMAssetHandle::Invalid();
		CMRenderer::CMDirectX::DXShaderSetType SetType = CMRenderer::CMDirectX::DXShaderSetType::INVALID;
	};
}