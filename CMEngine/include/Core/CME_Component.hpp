#pragma once

#include "Core/CME_AssetHandle.hpp"
#include "DirectX/CME_DXShaderData.hpp"
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

		[[nodiscard]] bool operator==(const CMTransformComponent& other) noexcept;
		[[nodiscard]] bool operator==(const CMCommon::CMTransform& transform) noexcept;
		[[nodiscard]] bool IsEqual(const CMTransformComponent& other) noexcept;
		[[nodiscard]] bool IsEqual(const CMCommon::CMTransform& transform) noexcept;
		[[nodiscard]] bool IsNearEqual(const CMTransformComponent& other, float epsilon = CMCommon::G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept;
		[[nodiscard]] bool IsNearEqual(const CMCommon::CMTransform& transform, float epsilon = CMCommon::G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept;

		CMCommon::CMTransform Transform;
	};

	struct CMRigidTransformComponent : public CMComponent
	{
		CMRigidTransformComponent(CMCommon::CMRigidTransform rigidTransform = CMCommon::CMRigidTransform()) noexcept;
		~CMRigidTransformComponent() = default;

		[[nodiscard]] bool operator==(const CMRigidTransformComponent& other) noexcept;
		[[nodiscard]] bool operator==(const CMCommon::CMRigidTransform& rigidTransform) noexcept;
		[[nodiscard]] bool IsEqual(const CMRigidTransformComponent& other) noexcept;
		[[nodiscard]] bool IsEqual(const CMCommon::CMRigidTransform& rigidTransform) noexcept;
		[[nodiscard]] bool IsNearEqual(const CMRigidTransformComponent& other, float epsilon = CMCommon::G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept;
		[[nodiscard]] bool IsNearEqual(const CMCommon::CMRigidTransform& rigidTransform, float epsilon = CMCommon::G_CMFLOAT_NEAR_EQUAL_EPSILON) noexcept;

		CMCommon::CMRigidTransform RigidTransform;
	};

	struct CMMeshComponent : public CMComponent
	{
		CMMeshComponent(
			CMCommon::CMTransform transform,
			Asset::CMAssetHandle handle,
			DirectXAPI::DX11::DXShaderSetType setType
		) noexcept;

		CMMeshComponent() = default;
		~CMMeshComponent() = default;

		CMCommon::CMTransform Transform = {};
		Asset::CMAssetHandle Handle = Asset::CMAssetHandle::Invalid();
		DirectXAPI::DX11::DXShaderSetType SetType = DirectXAPI::DX11::DXShaderSetType::INVALID;
	};

	using CMCameraComponent = CMRigidTransformComponent;
}