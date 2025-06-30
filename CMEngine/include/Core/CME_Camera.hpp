#pragma once

#include "CMC_Types.hpp"
#include <variant>

namespace CMEngine
{
	enum class CMProjectionType : int8_t
	{
		INVALID = -1,
		ORTHOGRAPHIC,
		PERSPECTIVE
	};

	struct CMOrthographicParams
	{
		inline constexpr CMOrthographicParams(
			float viewLeft,
			float viewRight,
			float viewBottom,
			float viewTop
		) noexcept;

		inline constexpr CMOrthographicParams(const CMCommon::CMRect& viewRect) noexcept;

		constexpr CMOrthographicParams() = default;
		~CMOrthographicParams() = default;

		CMCommon::CMRect View;
	};

	struct CMPerspectiveParams
	{
		inline constexpr CMPerspectiveParams(float vertFovDegrees) noexcept;

		constexpr CMPerspectiveParams() = default;
		~CMPerspectiveParams() = default;

		float VertFovDegrees = 45.0f;
	};

	struct CMCameraData
	{
		inline constexpr CMCameraData(
			const CMCommon::CMRigidTransform& rigidTransform,
			const CMOrthographicParams& orthographic,
			float nearZ,
			float farZ
		) noexcept;

		inline constexpr CMCameraData(
			const CMCommon::CMRigidTransform& rigidTransform,
			CMPerspectiveParams perspective,
			float nearZ,
			float farZ
		) noexcept;

		constexpr CMCameraData() = default;
		~CMCameraData() = default;

		CMCommon::CMRigidTransform RigidTransform;
		std::variant<CMOrthographicParams, CMPerspectiveParams> Projection;
		float NearZ = 0.5f;
		float FarZ = 1.0f;
	};

	inline constexpr CMOrthographicParams::CMOrthographicParams(
		float viewLeft,
		float viewRight,
		float viewBottom,
		float viewTop
	) noexcept
		: View(viewLeft, viewRight, viewBottom, viewTop)
	{
	}

	inline constexpr CMOrthographicParams::CMOrthographicParams(const CMCommon::CMRect& viewRect) noexcept
		: View(viewRect)
	{
	}

	inline constexpr CMPerspectiveParams::CMPerspectiveParams(float vertFovDegrees) noexcept
		: VertFovDegrees(vertFovDegrees)
	{
	}

	inline constexpr CMCameraData::CMCameraData(
		const CMCommon::CMRigidTransform& rigidTransform,
		const CMOrthographicParams& orthographic,
		float nearZ,
		float farZ
	) noexcept
		: RigidTransform(rigidTransform),
		  Projection(orthographic),
		  NearZ(nearZ),
		  FarZ(farZ)
	{
	}

	inline constexpr CMCameraData::CMCameraData(
		const CMCommon::CMRigidTransform& rigidTransform,
		CMPerspectiveParams perspective,
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