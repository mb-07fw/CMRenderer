#include "PCH.hpp"
#include "Platform/WinImpl/Platform_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
	PlatformConfig::PlatformConfig() noexcept
	{
		const wchar_t* pCmdLineWStr = GetCommandLineW();
		int numArgs = 0;

		const LPWSTR* pCmdLineArgsWStr = CommandLineToArgvW(pCmdLineWStr, &numArgs);

		if (numArgs == 1)
			return;

		constexpr std::wstring_view UseGraphicsDebuggerFlag = L"-fCM_ENGINE_USE_GRAPHICS_DEBUGGER";

		for (int i = 1; i < numArgs; ++i)
			if (UseGraphicsDebuggerFlag == pCmdLineArgsWStr[i])
				IsGraphicsDebugging = true;
	}

	SpdlogManager::SpdlogManager() noexcept
	{
		std::shared_ptr<spdlog::sinks::stdout_color_sink_st> pConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
		pConsoleSink->set_level(spdlog::level::info);
		pConsoleSink->set_pattern("[CMEngine] [%^%l%$] %v");

		std::shared_ptr<spdlog::sinks::callback_sink_st> pCallbackSink = std::make_shared<spdlog::sinks::callback_sink_st>(
			spdlog::custom_log_callback(std::bind(&SpdlogManager::ErrorCallback, this, std::placeholders::_1))
		);
		pCallbackSink->set_level(spdlog::level::err);

		CM_ENGINE_IF_DEBUG(std::string LogPath = "logs/cm_log_debug.txt");
		CM_ENGINE_IF_RELEASE(std::string LogPath = "logs/cm_log_release.txt");

		std::shared_ptr<spdlog::sinks::basic_file_sink_st> pFileSink = std::make_shared<spdlog::sinks::basic_file_sink_st>(LogPath, true);
		pFileSink->set_level(spdlog::level::trace);

		mP_Logger = std::make_shared<spdlog::logger>("CMENGINE_LOG", spdlog::sinks_init_list{ pConsoleSink, pFileSink, pCallbackSink });
		spdlog::set_default_logger(mP_Logger);

		CM_ENGINE_IF_DEBUG(spdlog::info("Debug build!!!"));
		spdlog::info("Working directory: {}", std::filesystem::current_path().generic_string());
	}

	void SpdlogManager::ErrorCallback(const spdlog::details::log_msg& msg) noexcept
	{
		for (const auto& sink : mP_Logger->sinks())
			sink->flush();

		std::cout << "Error message was posted. Terminating process.\n";
		std::exit(-1);
	}

	Platform::Platform() noexcept
		: m_Config(),
		  m_Graphics(m_Window, m_Config)
	{
	}

	bool Platform::Update() noexcept
	{
		if (m_Window.ShouldClose())
			return false;

		m_Window.Update();

		return true;
	}
}