#include "Core/PCH.hpp"
#include "Win/Win_Utility.hpp"

namespace CMEngine::Win::Utility
{
    [[nodiscard]] std::wstring TranslateError(DWORD errorCode) noexcept
    {
        if (errorCode == ERROR_SUCCESS)
            return L"NO_ERROR";

        wchar_t* pMessage = nullptr;
        DWORD msgLength = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPWSTR>(&pMessage),
            0,
            nullptr
        );

        if (msgLength == 0 || pMessage == nullptr)
            return L"UNKNOWN";

        std::wstring errorString(pMessage);

        LocalFree(pMessage);

        return errorString;
    }

    [[nodiscard]] std::wstring TranslateError(HRESULT hResult) noexcept
    {
        DWORD errorCode = HRESULT_CODE(hResult);
        
        return TranslateError(errorCode);
    }
}