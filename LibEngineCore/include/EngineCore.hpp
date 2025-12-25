#pragma once

#ifndef _WIN32
	#error This project is currently only usable on Windows, due to it's heavy usage of the glorious WinAPI, and no currently implemented fallback solution.
#endif

#if !defined(CM_DEBUG) && !defined(CM_RELEASE) && !defined(CM_DIST)
	#error An invalid build configuration has been set. This is problematic as code may be modified with the preprocessor depending on configuration.
#endif

#include "Event/EventSystem.hpp"
#include "Platform.hpp"
#include "Renderer.hpp"
#include "Asset/AssetManager.hpp"
#include "ECS/ECS.hpp"
#include "Scene/SceneManager.hpp"

namespace CMEngine
{
	class EngineCore
	{
	public:
		EngineCore() noexcept;
		~EngineCore() noexcept;
	
		EngineCore(const EngineCore&) = delete;
		EngineCore(EngineCore&&) = delete;
		EngineCore& operator=(const EngineCore&) = delete;
		EngineCore& operator=(EngineCore&&) = delete;
	public:
		void Update() noexcept;

		inline [[nodiscard]] Event::EventSystem& EventSystem() noexcept { return m_EventSystem; }
		inline [[nodiscard]] APlatform& Platform() noexcept { return m_Platform; }
		inline [[nodiscard]] ECS::ECS& ECS() noexcept { return m_ECS; }
		inline [[nodiscard]] Renderer::Renderer& Renderer() noexcept { return m_Renderer; }
		inline [[nodiscard]] Asset::AssetManager& AssetManager() noexcept { return m_AssetManager; }
		inline [[nodiscard]] Scene::SceneManager& SceneManager() noexcept { return m_SceneManager; }
	private:
		Event::EventSystem m_EventSystem;
		APlatform m_Platform;
		ECS::ECS m_ECS;
		Renderer::Renderer m_Renderer;
		Asset::AssetManager m_AssetManager;
		Scene::SceneManager m_SceneManager;
	};
}