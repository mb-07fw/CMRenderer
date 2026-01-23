#include "Engine.hpp"

#include <iostream>

namespace Engine
{
    Engine::Engine() noexcept
    {
        m_NativePlatform.CreateGraphicsContext(Platform::ApiType::Direct3D_11);

        std::cout << "Engine!\n";
    }

    Engine::~Engine() noexcept
    {
    }

    void Engine::Update() noexcept
    {
        m_NativePlatform.Window().Update();
    }
}