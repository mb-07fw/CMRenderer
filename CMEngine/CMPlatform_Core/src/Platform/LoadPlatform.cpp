#include "Platform/LoadPlatform.hpp"
#include "Platform/Platform.hpp"

#include <spdlog/spdlog.h>

namespace CMEngine::Platform
{
	static PlatformOutFunc sP_InitFunc = nullptr;
	static PlatformInFunc sP_ShutdownFunc = nullptr;
#ifdef _WIN32
	static HMODULE sP_PlatformWin = nullptr;

	[[nodiscard]] void Win32LoadPlatform(IPlatform*& pPlatform) noexcept
	{
		sP_PlatformWin = LoadLibraryEx(L"CMPlatform_WinImpl.dll", nullptr, 0);
	
		if (sP_PlatformWin == nullptr)
		{
			spdlog::error("Failed to load WinImpl platform. (Is it present with the executable?) Error Code: {}", GetLastError());
			exit(-1);
		}
	
		sP_InitFunc = (PlatformOutFunc)GetProcAddress(sP_PlatformWin, "WinImpl_Init");
	
		if (sP_InitFunc == nullptr)
		{
			spdlog::error("Failed to load Init function of WinImpl platform. Error Code: {}", GetLastError());
			exit(-1);
		}
	
		sP_ShutdownFunc = (PlatformInFunc)GetProcAddress(sP_PlatformWin, "WinImpl_Shutdown");
	
		if (sP_ShutdownFunc == nullptr)
		{
			spdlog::error("Failed to load Shutdown function of WinImpl platform. Error Code: {}", GetLastError());
			exit(-1);
		}
	
		pPlatform = sP_InitFunc();
	
		if (pPlatform == nullptr)
		{
			spdlog::error("Retrieved platform is nullptr. Error Code: {}", GetLastError());
			exit(-1);
		}

		//return gP_PlatformInstance;
	}

	void Win32Unload(IPlatform*& pPlatform) noexcept
	{
		if (sP_ShutdownFunc == nullptr)
		{
			spdlog::error("[Win32Unload] Shutdown function pointer is null.");
			exit(-1);
		}
		if (pPlatform == nullptr)
		{
			spdlog::error("[Win32Unload] Platform pointer is null.");
			exit(-1);
		}

		sP_ShutdownFunc(&pPlatform);

		sP_InitFunc = nullptr;
		sP_ShutdownFunc = nullptr;

		BOOL result = FreeLibrary(sP_PlatformWin);

		if (!result)
			spdlog::error("[Win32Unload] Failed to release platform library. Error Code: {}\n", GetLastError());

		sP_PlatformWin = nullptr;
	}
#endif
	
	[[nodiscard]] void LoadInterface(IPlatform*& pPlatform) noexcept
	{
	#ifdef _WIN32
		Win32LoadPlatform(pPlatform);
	#else
		spdlog::error("The current platform is currently un-supported. Terminating the program.\n");
		exit(-1);
	#endif
	}

	void Unload(IPlatform*& pPlatform) noexcept
	{
	#ifdef _WIN32
		Win32Unload(pPlatform);
	#else
		spdlog::error("The current platform is currently un-supported. Terminating the program.\n");
		exit(-1);
	#endif
	}
}