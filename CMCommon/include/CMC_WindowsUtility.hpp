#pragma once

#include <minwindef.h>
#include <string>

namespace CMRenderer::WindowsUtility
{
    constexpr [[nodiscard]] bool IsAlphabeticalVK(USHORT virtualKey);

    [[nodiscard]] std::wstring TranslateDWORDError(DWORD errorCode) noexcept;
    [[nodiscard]] std::wstring TranslateHRESULTError(HRESULT hResult) noexcept;

    constexpr [[nodiscard]] bool IsAlphabeticalVK(USHORT virtualKey)
    {
        return (virtualKey - 'A' < 26);
    }
}