#include "CMR_PCH.hpp"
#include "CMR_Camera.hpp"

namespace CMRenderer
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