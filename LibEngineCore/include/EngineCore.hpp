#pragma once

#ifndef _WIN32
	#error This project is currently only usable on Windows, due to it's heavy usage of the glorious WinAPI.
#endif

#if !defined(CM_DEBUG) && !defined(CM_RELEASE) && !defined(CM_DIST)
	#error An invalid build configuration has been set. This is problematic as code may be modified with the preprocessor depending on configuration. Continuing will result in unpredictable behavior.
#endif

#include "Platform.hpp"
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

		inline [[nodiscard]] APlatform& Platform() noexcept { return m_Platform; }
		inline [[nodiscard]] Asset::AssetManager& AssetManager() noexcept { return m_AssetManager; }
		inline [[nodiscard]] ECS::ECS& ECS() noexcept { return m_ECS; }
		inline [[nodiscard]] Scene::SceneManager& SceneManager() noexcept { return m_SceneManager; }
	private:
		APlatform m_Platform;
		Asset::AssetManager m_AssetManager;
		ECS::ECS m_ECS;
		Scene::SceneManager m_SceneManager;
	};
}

/* <------- Engine Roadmap ------->
 *
 * !! Done !!!
 * 
 * 1. Create a model using blender. 
 * 2. Load the model in C++ using Assimp. (understand the model format)
 * 3. Implement a basic AssetManager.
 * 
 * !!! In Progress !!!
 * 
 * 4. Implement a basic scene / scene graph structure.
 * 
 */