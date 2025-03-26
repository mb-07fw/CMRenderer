#pragma once

#include <minwindef.h>
#include <string>

namespace CMRenderer::WindowsUtility
{
    std::wstring TranslateDWORDError(const DWORD errorCode) noexcept;
    std::wstring TranslateDWORDError(const HRESULT hResult) noexcept;
}