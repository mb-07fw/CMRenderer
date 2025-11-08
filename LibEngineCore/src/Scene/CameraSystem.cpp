#include "PCH.hpp"
#include "Scene/CameraSystem.hpp"
#include "Component.hpp"
#include "Macros.hpp"

namespace CMEngine::Scene
{
	CameraSystem::CameraSystem(ECS::ECS& ecs) noexcept
		: m_ECS(ecs)
	{
	}

	void CameraSystem::SetMainCamera(ECS::Entity e) noexcept
	{
		m_MainCamera = e;
	}

	[[nodiscard]] View<CameraComponent> CameraSystem::GetCamera(ECS::Entity e) noexcept
	{
		return m_ECS.TryGetComponent<CameraComponent>(e);
	}
}