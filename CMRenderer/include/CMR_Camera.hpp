#pragma once

#include "CMC_Types.hpp"

namespace CMRenderer
{
	// Note : Only perspective is currently implied.
	struct CMCameraData
	{
		CMCameraData(
			CMCommon::CMRigidTransform rigidTransform,
			float vertFovDegrees,
			float nearZ,
			float farZ
		) noexcept;

		CMCameraData() = default;
		~CMCameraData() = default;

		CMCommon::CMRigidTransform RigidTransform;
		float VertFovDegrees = 45.0f;
		float NearZ = 0.5f;
		float FarZ = 1.0f;
	};
}