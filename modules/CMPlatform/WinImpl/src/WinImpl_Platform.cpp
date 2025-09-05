//#include "CMPlatform/WinImpl_PCH.hpp"
#include "WinImpl_Platform.hpp"

//namespace CMEngine::Platform::WinImpl
//{
//	WinImpl_Platform* gP_PlatformInstance = nullptr;
//
//	static void WinImpl_Platform_HandleError(const spdlog::details::log_msg& msg) noexcept
//	{
//		spdlog::info("(WinImpl_Platform) Received error: {}", msg.payload.data());
//		std::exit(-1);
//	}
//
//	static void WinImpl_Platform_InitLog() noexcept
//	{
//		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//		auto newLogger = spdlog::callback_logger_mt("WinImpl_Platform", spdlog::custom_log_callback(WinImpl_Platform_HandleError));
//		
//		newLogger->sinks().back()->set_level(spdlog::level::err);
//		newLogger->sinks().emplace_back(consoleSink);
//
//		spdlog::set_default_logger(newLogger);
//	}
//
//	void WinImpl_Platform_EnforceInstantiated() noexcept
//	{
//		if (gP_PlatformInstance != nullptr)
//			return;
//
//		spdlog::error("Attempted an operation on WinImpl_Platform before it was instantiated.");
//		exit(-1);
//	}
//
//	CM_DYNAMIC_LOAD IPlatform* WinImpl_Platform_Init()
//	{
//		if (gP_PlatformInstance != nullptr)
//			spdlog::error("(WinImpl_Init) Attempted to re-initialize the platform instance.");
//
//		WinImpl_Platform_InitLog();
//
//		gP_PlatformInstance = new WinImpl_Platform();
//
//		spdlog::info("(WinImpl_Init) Initialized platform.");
//
//		return gP_PlatformInstance;
//	}
//
//	CM_DYNAMIC_LOAD void WinImpl_Platform_Shutdown(IPlatform** ppPlatform)
//	{
//		if (gP_PlatformInstance == nullptr)
//		{
//			spdlog::error("(WinImpl_Shutdown) Attempted to shutdown the platform instance before it was initialized.");
//			std::exit(-1);
//		}
//
//		delete gP_PlatformInstance;
//
//		gP_PlatformInstance = nullptr;
//		*ppPlatform = nullptr;
//
//		spdlog::info("(WinImpl_Shutdown) Shutdown platform.");
//	}
//
//#pragma region Platform Functions
//	CM_DYNAMIC_LOAD void WinImpl_Platform_Update()
//	{
//		WinImpl_Platform_EnforceInstantiated();
//
//		gP_PlatformInstance->Impl_Update();
//	}
//#pragma endregion
//
//	WinImpl_Platform::WinImpl_Platform() noexcept
//		/* WARNING: Technically this is fine ONLY if the pointers to m_Window
//		 *			  and m_Graphics are used after they're constructed. (Not in
//		 *			  IPlatform's constructor). */
//		: IPlatform(
//			&m_Window,
//			&m_Graphics,
//			PlatformFuncTable(
//				WinImpl_Platform_Update
//			)
//		  ),
//		  m_Window(), /* Window MUST outlive Graphics due to Graphics' dependancy on Window. */
//		  m_Graphics(m_Window)
//	{
//		Impl_Init();
//	}
//
//	WinImpl_Platform::~WinImpl_Platform() noexcept
//	{
//		Impl_Shutdown();
//	}
//
//	void WinImpl_Platform::Impl_Init() noexcept
//	{
//	}
//
//	void WinImpl_Platform::Impl_Shutdown() noexcept
//	{
//	}
//
//	void WinImpl_Platform::Impl_Update() noexcept
//	{
//		m_Window.Impl_Update();
//		m_Graphics.Impl_Update();
//	}
//}