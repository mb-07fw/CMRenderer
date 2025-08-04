#pragma once

#include <variant>

#include "Common/Types.hpp"

namespace CMEngine::Core
{
	struct OrthographicParams
	{
		Common::Rect View;
	};

	struct PerspectiveParams
	{
		float VertFovDegrees = 0.0f;
	};

	enum class ProjectionType : int8_t
	{
		INVALID = -127,
		ORTHOGRAPHIC = 0,
		PERSPECTIVE
	};

	struct CameraData
	{
		using ProjectionTy = std::variant < OrthographicParams, PerspectiveParams>;

		inline constexpr CameraData(
			Common::RigidTransform rigidTransform,
			const OrthographicParams& orthographicParams,
			float nearZ,
			float farZ
		) noexcept;

		inline constexpr CameraData(
			Common::RigidTransform rigidTransform,
			PerspectiveParams perspectiveParams,
			float nearZ,
			float farZ
		) noexcept;

		CameraData() = default;
		~CameraData() = default;

		Common::RigidTransform RigidTransform;
		ProjectionTy Projection;
		float NearZ = 0.5f;
		float FarZ = 1.0f;
	};

	inline constexpr CameraData::CameraData(
		Common::RigidTransform rigidTransform,
		const OrthographicParams& orthographicParams,
		float nearZ,
		float farZ
	) noexcept
		: RigidTransform(rigidTransform),
		  Projection(orthographicParams),
		  NearZ(nearZ),
		  FarZ(farZ)
	{
	}

	inline constexpr CameraData::CameraData(
		Common::RigidTransform rigidTransform,
		PerspectiveParams perspective,
		float nearZ,
		float farZ
	) noexcept
		: RigidTransform(rigidTransform),
		  Projection(perspective),
		  NearZ(nearZ),
		  FarZ(farZ)
	{
	}
}