#pragma once

#ifndef _WIN32
	#error This project is currently only usable on Windows, due to it's heavy usage of the glorious WinAPI.
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "Dwrite.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "runtimeobject.lib") // Windows::Foundation::Initialize, RoInitialize, RoUninitialize

#include "Core/CME_WindowSettings.hpp"
#include "Core/CME_AssetManager.hpp"
#include "Core/CME_SceneManager.hpp"
#include "Windows/CME_WNWindow.hpp"
#include "DirectX/CME_DXContext.hpp"
#include "CMC_Logger.hpp"
#include "CMC_ECS.hpp"

namespace CMEngine
{
	class CMEngine
	{
	public:
		CMEngine() noexcept;
		~CMEngine() noexcept;
	public:
		void Run() noexcept;

		void UpdateWindow() noexcept;
		void Clear(CMCommon::NormColor normalizedColor) noexcept;
		void Present() noexcept;
	private:
		void OnWindowResize() noexcept;
		void ShowEngineControl(float deltaTime) noexcept;
	private:
		static constexpr std::wstring_view S_LIFETIME_LOG_FILE_NAME = L"./logs/CMEngineLifetime.log";
		CMCommon::CMLoggerWide m_EngineLogger;
		CMWindowSettings m_WindowSettings;
		WindowsAPI::CMWindow m_Window;
		DirectXAPI::DX11::DXContext m_Context;
		CMCommon::CMECS m_ECS;
		Asset::CMAssetManager m_AssetManager;
		CMSceneManager m_SceneManager;
	};
}