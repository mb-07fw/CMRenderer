#include "Core/CME_PCH.hpp"
#include "Core/CME_Camera.hpp"

namespace CMEngine
{
	CMCameraData::CMCameraData(
		CMCommon::CMRigidTransform rigidTransform,
		float vertFovDegrees,
		float nearZ,
		float farZ
	) noexcept
		: RigidTransform(rigidTransform),
		VertFovDegrees(vertFovDegrees),
		NearZ(nearZ),
		FarZ(farZ)
	{
	}
}