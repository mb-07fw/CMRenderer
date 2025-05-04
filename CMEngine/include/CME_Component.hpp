#pragma once

#include "CMC_Types.hpp"
#include "CMR_DXShaderData.hpp"

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
			const std::vector<float>& verticesRef,
			const std::vector<uint16_t>& indicesRef,
			CMRenderer::CMDirectX::DXShaderSetType setType
		) noexcept;

		CMMeshComponent() = default;
		~CMMeshComponent() = default;

		std::vector<float> Vertices;
		std::vector<uint16_t> Indices;

		CMCommon::CMTransform Transform = {};

		CMRenderer::CMDirectX::DXShaderSetType SetType = CMRenderer::CMDirectX::DXShaderSetType::INVALID;
	};
}