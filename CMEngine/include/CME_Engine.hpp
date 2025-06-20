#pragma once

#include "CMC_ECS.hpp"
#include "CMC_Logger.hpp"
#include "CMR_Renderer.hpp"
#include "CME_SceneManager.hpp"
#include "CME_AssetManager.hpp"

#include <memory>

namespace CMEngine
{
	class CMEngine
	{
	public:
		CMEngine() noexcept;
		~CMEngine() noexcept;
	public:
		void Run() noexcept;
	private:
		void ShowEngineControl(float deltaTime) noexcept;
	private:
		static constexpr std::wstring_view S_LIFETIME_LOG_FILE_NAME = L"./logs/CMEngineLifetime.log";
		CMCommon::CMLoggerWide m_EngineLogger;
		CMCommon::CMECS m_ECS;
		CMRenderer::CMRenderer m_Renderer;
		Asset::CMAssetManager m_AssetManager;
		CMSceneManager m_SceneManager;
	};
}