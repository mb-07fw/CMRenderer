#pragma once

#include "Export.hpp"
#include "Platform.hpp"
#include "Asset/AssetManager.hpp"
#include "Scene/SceneManager.hpp"
#include "ECS/ECS.hpp"

#include <memory>

namespace CMEngine
{
	class ILayer
	{
	public:
		virtual ~ILayer() = default;

		virtual void OnAttach() noexcept = 0;
		virtual void OnDetatch() noexcept = 0;
		virtual void OnUpdate() noexcept = 0;
	private:
	};

	class EditorLayer : public ILayer
	{
	public:
		EditorLayer(
			APlatform& platform,
			Asset::AssetManager& assetManager,
			Scene::SceneManager& sceneManager,
			ECS::ECS& ecs
		) noexcept;

		~EditorLayer() = default;

		virtual void OnAttach() noexcept override;
		virtual void OnDetatch() noexcept override;
		virtual void OnUpdate() noexcept override;
	private:
		APlatform& m_Platform;
		Asset::AssetManager& m_AssetManager;
		Scene::SceneManager& m_SceneManager;
		ECS::ECS& m_ECS;
		Scene::SceneID m_EditorScene = {};
		//std::unique_ptr<IUploadable> mP_CameraCB;
	};
}