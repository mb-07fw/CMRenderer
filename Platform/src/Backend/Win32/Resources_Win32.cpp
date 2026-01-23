#include "Resources_Win32.hpp"

namespace Platform::Backend::Win32
{
    [[nodiscard]] size_t Blob::Size() noexcept
    {
        if (!pBlob)
            return 0;

        return pBlob->GetBufferSize();
    }

    [[nodiscard]] void* Blob::Data() noexcept
    {
        if (!pBlob)
            return nullptr;

        return pBlob->GetBufferPointer();
    }
}