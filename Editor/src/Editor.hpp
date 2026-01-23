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
        Engine::Ref<Platform::IInputLayout> m_InputLayout;
        Engine::Ref<Platform::IVertexShader> m_VertexShader;
        Engine::Ref<Platform::IPixelShader> m_PixelShader;
        Engine::Ref<Platform::IVertexBuffer> m_VertexBuffer;
    };
}