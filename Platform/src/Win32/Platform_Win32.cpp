#include "Win32/Platform_Win32.hpp"
#include "Win32/Window_Win32.hpp"
#include "Win32/Context_Win32.hpp"
#include "Win32/PlatformOS_Win32.hpp"
#include "Common/Assert.hpp"

#include <iostream>
#include <string_view>

extern Platform::ILogger* gP_ActiveLogger = nullptr;

namespace Platform::Win32
{
    struct Platform::Impl
    {
        Impl() = default;
        ~Impl() = default;

        Win32::Window Window;
        Win32::Graphics::Context Context;
    };

    Platform::Platform() noexcept
    {
        /* ... manual command line parsing shenanigans ...  */
        const wchar_t* pCmdLineWStr = GetCommandLineW();
		int numArgs = 0;

		const LPWSTR* pCmdLineArgsWStr = CommandLineToArgvW(pCmdLineWStr, &numArgs);

		if (numArgs > 1)
        {
            constexpr std::wstring_view GraphicsDebuggerFlag = L"-fGRAPHICS_DEBUGGER";

            for (int i = 1; i < numArgs; ++i)
                if (GraphicsDebuggerFlag == pCmdLineArgsWStr[i])
                    m_Settings.IsGraphicsDebugging = true;

        }

        mP_Impl = new Impl();

        std::cout << "Platform_Win32!\n";
    }

    Platform::~Platform() noexcept
    {
        delete mP_Impl;
        mP_Impl = nullptr;

        std::cout << "Platform_Win32 Destroyed!\n";
    }

    void Platform::CreateGraphicsContext(Graphics::ApiType api) noexcept
    {
        ::HWND hWnd = mP_Impl->Window.Impl_hWnd();
        Win32::Graphics::Context& context = mP_Impl->Context;

        ASSERT(
            context.Impl_Create(api, m_Settings, hWnd),
            "(Platform_Win32) Failed to create a graphics api instance."
        );
    }

    [[nodiscard]] IWindow& Platform::Window() noexcept
    {
        return mP_Impl->Window;
    }

    [[nodiscard]] Graphics::IContext& Platform::Context() noexcept
    {
        return mP_Impl->Context;
    }
}