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

		[[nodiscard]] View<CameraComponent> GetCamera(ECS::Entity e) noexcept;
		inline [[nodiscard]] View<CameraComponent> GetMainCamera() noexcept { return m_ECS.TryGetComponent<CameraComponent>(m_MainCamera); }
		inline [[nodiscard]] ECS::Entity GetMainCameraEntity() const noexcept { return m_MainCamera; }
	private:
		ECS::ECS& m_ECS;
		ECS::Entity m_MainCamera;
	};
}