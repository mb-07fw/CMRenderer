#pragma once

#include "Export.hpp"

#include <cstdint>

namespace Platform
{
    /* Represents a platform-specific resource that is meant to be intrusively ref-counted. */
    struct PLATFORM_API IRefCounted
    {
        IRefCounted() = default;
        virtual ~IRefCounted() = default;

        virtual [[nodiscard]] size_t RefCount() const noexcept = 0;

        /* HUGE WARNING: You probably guessed it, but Release is meant to delete any derived instance.
        *
        *                   So if you ever thought to yourself,
        *
        *                   "... damn, why did everything explode... "
        *
        *                   You know exactly why.
        *
        *                   Yes future you, I am talking to you. You know what you did.
        */
        virtual void Release(uint32_t key = 0) noexcept = 0;

        virtual void AddRef(uint32_t key = 0) noexcept = 0;
        virtual [[nodiscard]] size_t DecRef(uint32_t key = 0) noexcept = 0;
    };
}