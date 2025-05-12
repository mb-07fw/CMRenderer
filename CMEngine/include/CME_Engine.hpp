#pragma once

#include "CMR_Renderer.hpp"
#include "CMC_ECS.hpp"
#include "CME_SceneManager.hpp"
#include "CMC_Logger.hpp"

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
		void ShowWindowControl() noexcept;
	private:
		static constexpr std::wstring_view S_LIFETIME_LOG_FILE_NAME = L"./logs/CMEngineLifetime.log";
		CMCommon::CMLoggerWide m_EngineLogger;
		CMCommon::CMECS m_ECS;
		CMRenderer::CMRenderer m_Renderer;
		CMSceneManager m_SceneManager;
	};
}