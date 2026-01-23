#include "Backend/Win32/Context_Win32.hpp"
#include "Backend/Win32/D3D/_11/API_D3D11.hpp"
#include "Common/Assert.hpp"
#include "Common/Cast.hpp"
#include "Common/RuntimeFailure.hpp"
#include "ILogger.hpp"

#define CONTEXT_FAILURE_IF(x, msg) RUNTIME_FAILURE_IF(x, msg, GetActiveLogger())

namespace Platform::Backend::Win32
{
    [[nodiscard]] bool Context::Impl_Create(
        ApiType api,
        const PlatformSettings& settings,
        ::HWND hWnd
    ) noexcept
    {
        return Create(api, settings, hWnd);
    }

    [[nodiscard]] IApi& Context::Api() noexcept
    {
        return GetApiInternal();
    }

    [[nodiscard]] bool Context::Create(
        ApiType api,
        const PlatformSettings& settings,
        void* pOSWindowHandle
    ) noexcept
    {
        if (mP_Api.get() != nullptr)
        {
            LogWarning(
                "(Context_Win32) Attempted to create an instance of "
                "a graphics api while another instance is already active."
            );

            return false;
        }

        CONTEXT_FAILURE_IF(pOSWindowHandle == nullptr, "(Context_Win32) Wtf are you doing?!");

        ::HWND hWnd = Reinterpret<::HWND>(pOSWindowHandle);

        switch (api)
        {
        case ApiType::Default:
            CreateDefaultApi(settings, hWnd);
            break;
        case ApiType::Direct3D_11:
            CreateDirect3D11Api(settings, hWnd);
            break;
        case ApiType::Direct3D_12:
            CreateDirect3D12Api();
            break;
        case ApiType::OpenGL:
            CreateOpenGLApi();
            break;
        default:
            CONTEXT_FAILURE_IF(true, "(Context_Win32) Unknown provided ApiType.");
            return false;
        }

        return true;
    }

    [[nodiscard]] IApi& Context::GetApiInternal() noexcept
    {
        CONTEXT_FAILURE_IF(
            mP_Api.get() == nullptr,
            "(Context_Win32) Attempted to retrieve a reference to an instance of the graphics api before it was created."
        );

        return *mP_Api;
    }

    void Context::CreateDefaultApi(const PlatformSettings& settings, ::HWND hWnd) noexcept
    {
        CreateDirect3D11Api(settings, hWnd);
    }

    void Context::CreateDirect3D11Api(const PlatformSettings& settings, ::HWND hWnd) noexcept
    {
        mP_Api = std::move(std::make_unique<D3D::_11::Api>(settings, hWnd));
    }

    void Context::CreateDirect3D12Api() noexcept
    {
        CONTEXT_FAILURE_IF(true, "(Context_Win32) Direct3D12 has yet to have been implemented.");
    }

    void Context::CreateOpenGLApi() noexcept
    {
        CONTEXT_FAILURE_IF(true, "(Context_Win32) OpenGL has yet to have been implemented.");
    }
}