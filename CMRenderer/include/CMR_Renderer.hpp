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

#pragma comment(lib, "CMDep_DearImGui.lib")

// Here to prevent the winnt.h #error "No Target Architecture"... idk why it's happening anyway tho...
#include <Windows.h> 

#include <string_view>

#include "CMC_Macros.hpp"
#include "CMC_Logger.hpp"

#include "CMR_RendererSettings.hpp"
#include "CMR_Window.hpp"
#include "CMR_DXContext.hpp"
//#include "CMR_ShapeQueue.hpp"

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
		
		void UpdateWindow() noexcept;

		void Clear(CMCommon::NormColor normalizedColor) noexcept;
		void Present() noexcept;

		inline [[nodiscard]] bool IsInitialized() const noexcept { return m_Initialized; }
		inline [[nodiscard]] bool IsShutdown() const noexcept { return m_Shutdown; }

		inline [[nodiscard]] CMWindow& Window() noexcept { return m_Window; }
		inline [[nodiscard]] CMDirectX::DXContext& RenderContext() noexcept { return m_Context; }
	private:
		void OnWindowResize() noexcept;
	private:
		static constexpr std::wstring_view S_LIFETIME_LOG_FILE_NAME = L"../CMRenderer/logs/CMRendererLifetime.log";
		CMCommon::CMLoggerWide m_CMLogger;
		CMRendererSettings m_Settings;
		CMWindow m_Window;
		CMDirectX::DXContext m_Context;
		bool m_Initialized = false;
		bool m_Shutdown = false;
	};
}
