#include "Editor.hpp"

#include "Common/Cast.hpp"
#include "Common/Math.hpp"

#include <iostream>
#include <chrono>
#include <thread>

namespace Editor
{
    Editor::Editor() noexcept
    {
        std::cout << "Editor!\n";
    }

    Editor::~Editor() noexcept
    {
    }

    void Editor::Run() noexcept
    {
        using namespace Engine;

        Platform::Native& platform = m_Engine.Platform();
        Platform::IWindow& window = platform.Window();
        Platform::Graphics::IContext& context = platform.Context();
        Platform::Graphics::IApi& api = context.Api();

        while (window.IsRunning())
        {
            using Clock = std::chrono::steady_clock;

            auto startTime = Clock::now();

            m_Engine.Update();

            constexpr float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

            api.Clear(ClearColor);

            api.Present();

            constexpr auto TargetFrameTime = 1000.0f / 60.0f;

            auto endTime = Clock::now();

            using Milliseconds = std::chrono::duration<float, std::milli>;

            float frameMillis = std::chrono::duration_cast<Milliseconds>(endTime - startTime).count();

            float remainingMillis = TargetFrameTime - frameMillis;

            std::this_thread::sleep_for(Milliseconds(remainingMillis));
        }
    }
}