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