#pragma once

#include "IRefCounted.hpp"
#include "Common/Assert.hpp"

#include <atomic>
#include <cstdint>

namespace Platform::Backend
{
    /* makeshift ownership contract:
     *   - calling AddRef / DecRef / Release directly is forbidden.
     *       (punishible by pouring water on your computer -- use Ref<T> to manage lifetime or else)
     * 
     *   - calling Release invalidates the object immediately. */
	struct RefCounted
	{
        /* This literally has no meaning. It just needs to be unique.
         * Ves. this is evil... but it's also really funny.
         * Obviously this doesn't prevent any misuse, its more like a contraceptive. */
        static constexpr uint32_t S_SuperSecretSpecialRefCountOpKey = ((2 * 16) << 26) + 43;

        RefCounted() = default;
        virtual ~RefCounted() = default;

        inline [[nodiscard]] size_t RefCountImpl() const noexcept
        {
            return m_RefCount.load(std::memory_order_acquire);
        }

        inline void ReleaseImpl(uint32_t key) noexcept
        {
            ASSERT(
                key == S_SuperSecretSpecialRefCountOpKey,
                "Release was called directly (outside of Ref). "
                "Please proceed to hydrate your computer accordingly."
            );

            /* (fetch_sub returns the value before subtraction) */
            if (DecRefImpl(S_SuperSecretSpecialRefCountOpKey) == 1)
                delete this; // Note: Very important this happens on the platform .dll side.
        }
    protected:
        inline void AddRefImpl(uint32_t key = 0) noexcept
        {
            ASSERT(
                key == S_SuperSecretSpecialRefCountOpKey,
                "AddRefImpl was called directly (outside of Ref). "
                "Please proceed to hydrate your computer accordingly."
            );

            m_RefCount.fetch_add(1, std::memory_order_relaxed);
        }

        inline [[nodiscard]] size_t DecRefImpl(uint32_t key = 0) noexcept
        {
            ASSERT(
                key == S_SuperSecretSpecialRefCountOpKey,
                "DecRefImpl was called directly (outside of Ref). "
                "Please proceed to hydrate your computer accordingly."
            );

            return m_RefCount.fetch_sub(1, std::memory_order_acq_rel);
        }
    private:
        std::atomic_size_t m_RefCount = 0;
	};
}