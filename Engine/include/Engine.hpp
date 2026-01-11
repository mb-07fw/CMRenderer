#pragma once

#include "Platform.hpp"

namespace Engine
{
    class Engine
    {
    public:
        Engine() noexcept;
        ~Engine() noexcept;

        void Update() noexcept;

        inline [[nodiscard]] Platform::Native& Platform() noexcept { return m_NativePlatform; }
    private:
        Platform::Native m_NativePlatform;
    };
}