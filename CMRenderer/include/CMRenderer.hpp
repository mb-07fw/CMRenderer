#pragma once

#ifndef _WIN32
	#error This project is currently only usable on Windows, due to it's heavy usage of the WinAPI.
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "Dwrite.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "runtimeobject.lib") // Windows::Foundation::Initialize, RoInitialize, RoUninitialize

#pragma comment(lib, "DearImGUI_CMDep.lib")

#include "Core/CMMacros.hpp"
#include "Internal/Utility/CMLogger.hpp"
#include "Internal/CMRendererSettings.hpp"
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
		static constexpr std::wstring_view S_LIFETIME_LOG_FILE_NAME = L"logs/CMRendererLifetime.log";
		Utility::CMLoggerWide m_CMLogger;
		CMRendererSettings m_Settings;
		CMWindow m_Window;
		CMDirectX::DXContext m_RenderContext;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}
