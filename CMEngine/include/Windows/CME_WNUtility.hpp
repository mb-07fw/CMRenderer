#pragma once

#include <minwindef.h>
#include <string>

namespace CMEngine::WindowsAPI::Utility
{
    [[nodiscard]] std::wstring TranslateError(DWORD errorCode) noexcept;
    [[nodiscard]] std::wstring TranslateError(HRESULT hResult) noexcept;

    constexpr [[nodiscard]] bool IsAlphabeticalVK(USHORT virtualKey)
    {
        return (virtualKey - 'A' < 26);
    }
}