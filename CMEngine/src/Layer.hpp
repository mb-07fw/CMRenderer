#pragma once

#include "Export.hpp"
#include "Platform.hpp"
#include "Asset/AssetManager.hpp"
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
			ECS::ECS& ecs
		) noexcept;

		~EditorLayer() = default;

		virtual void OnAttach() noexcept override;
		virtual void OnDetatch() noexcept override;
		virtual void OnUpdate() noexcept override;
	private:
		APlatform& m_Platform;
		Asset::AssetManager& m_AssetManager;
		ECS::ECS& m_ECS;
		std::unique_ptr<IUploadable> mP_CameraCB;
		ECS::Entity m_Camera;
	};
}