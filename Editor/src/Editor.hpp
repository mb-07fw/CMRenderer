#pragma once

#include "Engine.hpp"

namespace Editor
{
    class Editor
    {
    public:
        Editor() noexcept;
        ~Editor() noexcept;

        void Run() noexcept;
    private:
        Engine::Engine m_Engine;
    };
}