#pragma once

#include "Core/CME_AssetManager.hpp"
#include "Core/CME_Component.hpp"
#include "DirectX/CME_DXRenderer.hpp"
#include "CMC_ECS.hpp"

#include <functional>

namespace CMEngine
{
	enum class CMStockSceneType
	{
		TEST_SCENE
	};

	class ICMScene
	{
	public:
		ICMScene(
			CMCommon::CMLoggerWide& logger,
			CMCommon::CMECS& ecs,
			Asset::CMAssetManager& assetManager,
			DirectXAPI::DX11::DXRenderer& renderer,
			std::function<void(float)> onUpdateFunc
		) noexcept;

		virtual ~ICMScene() = default;
	public:
		virtual void OnStart() noexcept = 0;
		virtual void OnEnd() noexcept = 0;

		void OnUpdate(float deltaTime) noexcept;
	protected:
		CMCommon::CMLoggerWide& m_Logger;
		CMCommon::CMECS& m_ECS;
		Asset::CMAssetManager& m_AssetManager;
		DirectXAPI::DX11::DXRenderer& m_Renderer;
	private:
		std::function<void(float)> m_OnUpdateFunc;
	};

	class ICMStockScene : public ICMScene
	{
	public:
		using ICMScene::ICMScene;
		virtual ~ICMStockScene() = default;
	};

	class CMTestScene : public ICMStockScene
	{
	public:
		CMTestScene(
			CMCommon::CMLoggerWide& logger,
			CMCommon::CMECS& ecs,
			Asset::CMAssetManager& assetManager,
			DirectXAPI::DX11::DXRenderer& renderer
		) noexcept;

		~CMTestScene() = default;
	public:
		virtual void OnStart() noexcept override;
		virtual void OnEnd() noexcept override;
		void OnUpdate(float deltaTime) noexcept;
	private:
		void ShowMeshWindow(CMCommon::CMTransform& meshTransform) noexcept;
		void ShowCameraWindow(CMCommon::CMRigidTransform& cameraTransform) noexcept;
	private:
		CMCommon::CMECSEntity m_CameraEntity = {};
		CMCommon::CMECSEntity m_MeshEntity = {};
		CMCommon::CMRigidTransform m_PreviousCameraTransform;
		CMCommon::CMTransform m_PreviousMeshTransform;
	};
}