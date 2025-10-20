#pragma once

#include "ECS/ECS.hpp"
#include "Component.hpp"

#include <vector>

namespace CMEngine::Scene
{
	class CameraSystem
	{
	public:
		CameraSystem(ECS::ECS& ecs) noexcept;
		~CameraSystem() = default;
	public:
		void SetMainCamera(ECS::Entity e) noexcept;

		[[nodiscard]] CameraComponent* GetCamera(ECS::Entity e) noexcept;
	private:
		ECS::ECS& m_ECS;
		ECS::Entity m_MainCamera;
	};
}