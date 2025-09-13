#pragma once

#ifndef _WIN32
	#error This project is currently only usable on Windows, due to it's heavy usage of the glorious WinAPI.
#endif

#if !defined(CM_DEBUG) && !defined(CM_RELEASE) && !defined(CM_DIST)
	#error An invalid build configuration has been set. This is problematic as code may be modified with the preprocessor depending on configuration. Continuing will result in unpredictable behavior.
#endif

#include "Export.hpp"
#include "Types.hpp"
#include "Platform.hpp"

namespace CMEngine
{
	class CM_ENGINE_API Engine
	{
	public:
		Engine() noexcept;
		~Engine() = default;
	
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
	public:
		void Run() noexcept;
	private:
		PlatformAlias m_Platform;
	};
}

/* <------- Engine Roadmap ------->
 * 
 * 1. Create a model using blender.
 * 
 *		i)  
 * 
 * 2. Load the model in C++ using Assimp. (understand the model format)
 * 3. Implement a basic scene / scene graph structure.
 * 4. Use an ECS to implement a Game Entity system.
 * 
 */