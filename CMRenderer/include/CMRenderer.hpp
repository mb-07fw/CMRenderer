#pragma once

#ifndef _WIN32
	#error This project is currently only usable on Windows, due to it's heavy usage of the WinAPI.
#endif

#ifndef _MSC_VER
	#error This project is currently only usable on MVSC compilers (Visual Studio) do to it's use of __debugbreak().
#endif

#if defined(CM_NO_DIRECTX) && defined(CM_NO_OPENGL) && defined(CM_NO_VULKAN)
	#error All Implemented graphics API's have been defined to be excluded, which is invalid as CMRenderer needs at least one of it's implementations. (e.g., OpenGL, DirectX, OpenGL)
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Internal/CMRendererSettings.hpp"
#include "Internal/CMLogger.hpp"
#include "Internal/CMWindow.hpp"
#include "Internal/DirectX/DXContext.hpp"

namespace CMRenderer
{
	class CMRenderer
	{
	public:
		/* The constructor is defined in the header file so previously defined macros such as CM_NO_DIRECTX are visible. */
		inline CMRenderer(CMRendererSettings settings) noexcept;
		~CMRenderer() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;
		
		void Run() noexcept;
	private:
		static constexpr std::wstring_view S_LIFETIME_LOG_FILE_NAME = L"logs/CMRendererLifetime.log";
		CMRendererSettings m_Settings;
		CMLoggerWide m_CMLogger;
		CMWindow m_Window;
		DirectX::DXContext m_RenderContext;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};



	inline CMRenderer::CMRenderer(CMRendererSettings settings) noexcept
		: m_Settings(settings), m_CMLogger(S_LIFETIME_LOG_FILE_NAME), m_Window(m_Settings, m_CMLogger),
		  m_RenderContext(m_CMLogger)
	{
#ifdef CM_NO_DIRECTX
		m_CMLogger.LogInfo(L"CMRenderer [()] | DirectX has been defined to be excluded.\n");
#endif

#ifdef CM_NO_OPENGL
		m_CMLogger.LogInfo(L"CMRenderer [()] | OpenGL has been defined to be excluded.\n");
#endif

#ifdef CM_NO_VULKAN
		m_CMLogger.LogInfo(L"CMRenderer [()] | Vulkan has been defined to be excluded.\n");
#endif

		m_CMLogger.LogInfo(L"CMRenderer [()] | CMRenderer : Initialized CMRenderer.\n");
	}
}