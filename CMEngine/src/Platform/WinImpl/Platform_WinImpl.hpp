#pragma once

#include "Export.hpp"
#include "Platform/Core/IPlatform.hpp"
#include "Platform/WinImpl/Window_WinImpl.hpp"
#include "Platform/WinImpl/Graphics_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
	/* Stores metadata specific to the current platform. */
	struct PlatformConfig
	{
		PlatformConfig() noexcept;
		~PlatformConfig() = default;
		
		bool IsGraphicsDebugging = false;
	};

	class SpdlogManager
	{
	public:
		SpdlogManager() noexcept;
		~SpdlogManager() = default;
	private:
		void ErrorCallback(const spdlog::details::log_msg& msg) noexcept;
	private:
		std::shared_ptr<spdlog::logger> mP_Logger;
	};

	class CM_ENGINE_API Platform : public IPlatform
	{
	public:
		Platform() noexcept;
		~Platform() = default;

		virtual bool Update() noexcept override;
		inline virtual [[nodiscard]] bool IsRunning() const noexcept override { return !m_Window.ShouldClose(); }
	private:
		const PlatformConfig m_Config;
		SpdlogManager m_SpdlogInitializer;
		Window m_Window;
		Graphics m_Graphics;
	};
}