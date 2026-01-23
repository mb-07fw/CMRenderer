#pragma once

#include "Backend/Win32/Types_Win32.hpp"
#include "Backend/RefCounted.hpp"
#include "ImplRefCounted.hpp"
#include "IApiResources.hpp"

#include <cstdint>
#include <d3dcommon.h>

namespace Platform::Backend::Win32
{
    struct Blob final : public IBlob, public RefCounted
    {
        Blob() = default;
        ~Blob() = default;

        impl_ref_count;

        virtual [[nodiscard]] size_t Size() noexcept override;
        virtual [[nodiscard]] OpaquePtr Data() noexcept override;

        ComPtr<::ID3DBlob> pBlob;
    };
}

#include "ImplRefCountedUndef.hpp"