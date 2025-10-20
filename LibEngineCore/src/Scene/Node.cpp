#include "PCH.hpp"
#include "Scene/Node.hpp"

namespace CMEngine::Scene::Node
{
	[[nodiscard]] Float3 Camera3D::GetOrigin() const noexcept
	{
		return m_Camera.Data.Origin;
	}

	[[nodiscard]] void Camera3D::UpdateOrigin(Float3 newOrigin) noexcept
	{
		/* Update view matrix is origin changes... */
		if (!m_Camera.Data.Origin.IsNearEqual(newOrigin))
			m_Camera.CreateViewMatrix();
	}

}