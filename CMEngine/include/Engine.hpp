#pragma once

#ifndef _WIN32
	#error This project is currently only usable on Windows, due to it's heavy usage of the glorious WinAPI.
#endif

#if !defined(CM_DEBUG) && !defined(CM_RELEASE) && !defined(CM_DIST)
	#error An invalid build configuration has been set. This is problematic as code may be modified with the preprocessor depending on configuration. Continuing will result in unpredictable behavior.
#endif

#include "Engine_Export.hpp"
#include "CMPlatform.hpp"

namespace CMEngine
{
	//class CM_API CMEngine
	//{
	//#ifdef _WIN32
	//	using NativeStringView = std::wstring_view;
	//#else
	//	using NativeStringView = std::string_view;
	//#endif
	//public:
	//	CMEngine() noexcept;
	//	~CMEngine() noexcept;

	//	CMEngine(const CMEngine& other) noexcept = delete;
	//	CMEngine& operator=(const CMEngine& other) noexcept = delete;
	//public:
	//	void Run() noexcept;
	//private:
	//	void Init() noexcept;
	//	void Shutdown() noexcept;
	//private:
	//	//Platform::IPlatform* mP_Platform = nullptr;
	//	bool m_Initialized = false;
	//	bool m_Shutdown = false;
	//};

	class CM_ENGINE_API Engine
	{
	public:
		Engine() noexcept;
		~Engine() = default;
	
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
	private:
		void CreatePlatform() noexcept;
	private:
	};
}

//
//#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "D2D1.lib")
//#pragma comment(lib, "Dwrite.lib")
//#pragma comment(lib, "DXGI.lib")
//#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "runtimeobject.lib") // Windows::Foundation::Initialize, RoInitialize, RoUninitialize
//
//#include "Core/AssetManager.hpp"
//#include "Core/SceneManager.hpp"
//#include "Core/LayerStack.hpp"
//#include "Core/ECS.hpp"
//#include "Common/Logger.hpp"
//#include "Common/MetaArena.hpp"
//#include "Win/Win_Window.hpp"
//#include "DX/DX11/DX11_Renderer.hpp"
//
///* TODO: (for the big future)
// *
// *   1 (or 2) - Make CMEngine a shared library!
// *   2 (or 1) - Add OpenGL as another backend renderer!
// *   
// */
//
//namespace CMEngine
//{
//	class CMEngine
//	{
//	public:
//		CMEngine() noexcept;
//		~CMEngine() noexcept;
//
//		CMEngine(const CMEngine& other) noexcept = delete;
//		CMEngine& operator=(const CMEngine& other) noexcept = delete;
//	public:
//		void Init() noexcept;
//		void Shutdown() noexcept;
//
//		void Run() noexcept;
//
//		void UpdateWindow() noexcept;
//		void Clear(Common::NormColor normalizedColor) noexcept;
//		void Present() noexcept;
//	private:
//		void OnWindowResize() noexcept;
//		void ShowEngineControl(float deltaTime) noexcept;
//	private:
//		static constexpr std::wstring_view S_LIFETIME_LOG_FILE_NAME = L"./logs/CMEngineLifetime.log";
//		Common::LoggerWide m_EngineLogger;
//		Common::MetaArena m_EngineHeap;
//		Win::WindowSettings m_WindowSettings;
//		Win::Window m_Window;
//		DX::DX11::Renderer m_Renderer;
//		Core::Asset::AssetManager m_AssetManager;
//		Core::ECS m_ECS;
//		Core::SceneManager m_SceneManager;
//		Core::LayerStack m_LayerStack;
//		bool m_Initialized = false;
//		bool m_Shutdown = false;
//	};
//}