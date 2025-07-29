#pragma once

#include "Core/AssetManager.hpp"
#include "Core/Component.hpp"
#include "Core/ECS.hpp"
#include "DX/DX11/DX11_Renderer.hpp"

#include <functional>

namespace CMEngine::Core
{
	enum class StockSceneType
	{
		TEST_SCENE
	};

	class IScene
	{
	public:
		IScene(
			Common::LoggerWide& logger,
			ECS& ecs,
			Asset::AssetManager& assetManager,
			DX::DX11::Renderer& renderer,
			std::function<void(float)> onUpdateFunc
		) noexcept;

		virtual ~IScene() = default;
	public:
		virtual void OnStart() noexcept = 0;
		virtual void OnEnd() noexcept = 0;

		void OnUpdate(float deltaTime) noexcept;
	protected:
		Common::LoggerWide& m_Logger;
		Core::ECS& m_ECS;
		Asset::AssetManager& m_AssetManager;
		DX::DX11::Renderer& m_Renderer;
	private:
		std::function<void(float)> m_OnUpdateFunc;
	};

	class IStockScene : public IScene
	{
	public:
		using IScene::IScene;
		virtual ~IStockScene() = default;
	};

	class TestScene : public IStockScene
	{
	public:
		TestScene(
			Common::LoggerWide& logger,
			Core::ECS& ecs,
			Asset::AssetManager& assetManager,
			DX::DX11::Renderer& renderer
		) noexcept;

		~TestScene() = default;
	public:
		virtual void OnStart() noexcept override;
		virtual void OnEnd() noexcept override;
		void OnUpdate(float deltaTime) noexcept;
	private:
		void ShowMeshWindow(Common::Transform& meshTransform) noexcept;
		void ShowCameraWindow(Common::RigidTransform& cameraTransform) noexcept;
	private:
		Core::ECSEntity m_CameraEntity = {};
		Core::ECSEntity m_MeshEntity = {};
		Common::RigidTransform m_PreviousCameraTransform;
		Common::Transform m_PreviousMeshTransform;
	};
}