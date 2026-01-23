#include "Backend/Win32/Platform_Win32.hpp"
#include "Backend/Win32/Window_Win32.hpp"
#include "Backend/Win32/Context_Win32.hpp"
#include "Backend/Win32/PlatformOS_Win32.hpp"
#include "Backend/Win32/D3D/_11/Api_D3D11.hpp"
#include "Common/Assert.hpp"

#include <functional>
#include <iostream>
#include <string_view>

namespace Platform::Backend::Win32
{
    struct Platform::Impl
    {
        Impl() = default;
        ~Impl() = default;

        Win32::Window Window;
        Win32::Context Context;
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

    void Platform::CreateGraphicsContext(ApiType api) noexcept
    {
        ::HWND hWnd = mP_Impl->Window.Impl_hWnd();
        Win32::Context& context = mP_Impl->Context;
        Win32::Window& window = mP_Impl->Window;

        ASSERT(
            context.Impl_Create(api, m_Settings, hWnd),
            "(Platform_Win32) Failed to create a graphics api instance."
        );

        auto onResizeFunc = [](
            uint32_t width,
            uint32_t height,
            void* pUserData)
        {
            auto& platform = *Reinterpret<Platform*>(pUserData);

            auto& api = platform.Context().Api();

            /* TODO: Make window resizing deferred... */
            api.OnWindowResize(width, height);
        };

        window.SetWindowResizeCallback(onResizeFunc, this);
    }

    [[nodiscard]] IWindow& Platform::Window() noexcept
    {
        return mP_Impl->Window;
    }

    [[nodiscard]] IContext& Platform::Context() noexcept
    {
        return mP_Impl->Context;
    }

    [[nodiscard]] IApi& Platform::GetGraphicsApi() noexcept
    {
        return Context().Api();
    }
    
    [[nodiscard]] IApiFactory& Platform::GetGraphicsApiFactory() noexcept
    {
        return Context().Api().Factory();
    }
}