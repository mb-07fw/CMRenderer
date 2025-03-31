#pragma once

#ifndef _WIN32
	#error This project is currently only usable on Windows, due to it's heavy usage of the WinAPI.
#endif

#if defined(CM_NO_DIRECTX) && defined(CM_NO_OPENGL) && defined(CM_NO_VULKAN)
	#error All Implemented graphics API's have been defined to be excluded, which is invalid as CMRenderer needs at least one of it's implementations. (e.g., OpenGL, DirectX, OpenGL)
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Core/CMMacros.hpp"
#include "Internal/CMRendererSettings.hpp"
#include "Internal/CMLogger.hpp"
#include "Internal/CMWindow.hpp"
#include "Internal/DirectX/DXContext.hpp"

namespace CMRenderer
{
	class CMRenderer
	{
	public:
		CMRenderer(CMRendererSettings settings) noexcept;
		~CMRenderer() noexcept;
	public:
		void Init() noexcept;
		void Shutdown() noexcept;
		
		void Run() noexcept;
	private:
		//void Suspend() noexcept;
	private:
		static constexpr std::wstring_view S_LIFETIME_LOG_FILE_NAME = L"logs/CMRendererLifetime.log";
		CMRendererSettings m_Settings;
		CMLoggerWide m_CMLogger;
		CMWindow m_Window;
		CMDirectX::DXContext m_RenderContext;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}
