#pragma once

#include "CMR_RendererSettings.hpp"
#include "CMC_ECS.hpp"
#include "CMR_Renderer.hpp" // TODO : Make a renderer command queue.
#include "CME_Component.hpp"

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
			CMCommon::CMLoggerWide& engineLoggerRef,
			CMCommon::CMECS& ECSRef,
			CMRenderer::CMRenderer& rendererRef,
			std::function<void(float)> onUpdateFunc
		) noexcept;
		virtual ~ICMScene() = default;
	public:
		virtual void OnStart() noexcept = 0;
		virtual void OnEnd() noexcept = 0;

		void OnUpdate(float deltaTime) noexcept;
	protected:
		CMCommon::CMLoggerWide& m_EngineLoggerRef;
		CMCommon::CMECS& m_ECSRef;
		CMRenderer::CMRenderer& m_RendererRef;
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
			CMCommon::CMLoggerWide& engineLoggerRef,
			CMCommon::CMECS& ECSRef,
			CMRenderer::CMRenderer& rendererRef
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