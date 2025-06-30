#pragma once

#include "Core/CME_Layer.hpp"
#include "Core/CME_AssetManager.hpp"
#include "Core/CME_SceneManager.hpp"
#include "CMC_ECS.hpp"
#include "CMC_MetaArena.hpp"

namespace CMEngine
{
	class CMEditorLayer : public ICMLayer
	{
	public:
		CMEditorLayer(
			CMCommon::CMLoggerWide& logger,
			DirectXAPI::DX11::DXRenderer& renderer,
			CMCommon::CMECS& ecs,
			Asset::CMAssetManager& assetManager,
			CMSceneManager& sceneManager,
			CMCommon::CMMetaArena& engineHeap
		) noexcept;

		~CMEditorLayer() = default;
	public:
		virtual void OnAttach() noexcept override;
		virtual void OnDetach() noexcept override;
		virtual void OnUpdate(float deltaTime) noexcept override;
	private:
		CMCommon::CMECS& m_ECS;
		Asset::CMAssetManager& m_AssetManager;
		CMSceneManager& m_SceneManager;
		CMCommon::CMMetaArena& m_EngineHeap;
		CMCommon::CMECSEntity m_GUIEntity;
	};
}