#pragma once

#include "CMC_ECS.hpp"
#include "CME_Scene.hpp"

namespace CMEngine
{
	class CMSceneManager
	{
	public:
		CMSceneManager(
			CMCommon::CMLoggerWide& engineLifetimeLoggerRef,
			CMCommon::CMECS& cmECSRef,
			CMRenderer::CMRenderer& rendererRef
		) noexcept;
		~CMSceneManager() = default;
	public:
		void UpdateActiveScene(float deltaTime) noexcept;
		void TransitionScene(CMStockSceneType type) noexcept;
	private:
		CMCommon::CMLoggerWide& m_EngineLoggerRef;
		CMCommon::CMECS& m_ECSRef;
		CMRenderer::CMRenderer& m_RendererRef; // Note to self : Stop being lazy and make this a command queue.
		CMTestScene m_TestScene;
		//std::vector<ICMScene*> m_Scenes; // Change this to shared_ptr's or arena allocated scenes.
		ICMScene* mP_ActiveScene = nullptr;
		bool m_StockSceneActive = false;
	};
}