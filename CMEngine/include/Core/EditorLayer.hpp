#pragma once

#include "Core/Layer.hpp"
#include "Core/AssetManager.hpp"
#include "Core/SceneManager.hpp"
#include "Core/ECS.hpp"
#include "Common/MetaArena.hpp"

namespace CMEngine::Core
{
	class EditorLayer : public ILayer
	{
	public:
		EditorLayer(
			Common::LoggerWide& logger,
			DX::DX11::Renderer& renderer,
			Win::Window& window,
			ECS& ecs,
			Asset::AssetManager& assetManager,
			SceneManager& sceneManager,
			Common::MetaArena& engineHeap
		) noexcept;

		~EditorLayer() = default;
	public:
		virtual void OnAttach() noexcept override;
		virtual void OnDetach() noexcept override;
		virtual void OnUpdate(float deltaTime) noexcept override;

		void ShowMeshWindow(Common::Transform& meshTransform) noexcept;
		void ShowCameraWindow(Common::RigidTransform& cameraTransform) noexcept;
	private:
		ECS& m_ECS;
		Asset::AssetManager& m_AssetManager;
		SceneManager& m_SceneManager;
		Common::MetaArena& m_EngineHeap;
		Core::ECSEntity m_CameraEntity;
		Core::ECSEntity m_GUIEntity;
		Common::RigidTransform m_PreviousCameraTransform;
		Common::Transform m_PreviousMeshTransform;
		DX::DX11::DrawDescriptor m_DrawDescriptor;
		float m_InstanceRadius = 1.0f;
	};
}