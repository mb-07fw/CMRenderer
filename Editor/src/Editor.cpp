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

        auto& assetManager = m_Engine.AssetManager();
        auto& platform = m_Engine.Platform();
        auto& context = platform.Context();
        auto& api = context.Api();
        auto& factory = api.Factory();
        
        using namespace Engine;

        constexpr std::string_view FilePathBasicVS = ENGINE_RESOURCES_SHADERS_DIR "BasicVS.hlsl";
        constexpr std::string_view FilePathBasicPS = ENGINE_RESOURCES_SHADERS_DIR "BasicPS.hlsl";

        Asset::Result result;

        Asset::AssetID vsID = assetManager.LoadShader(FilePathBasicVS);
        Asset::AssetID psID = assetManager.LoadShader(FilePathBasicPS);

        ASSERT(vsID.IsValid(), "oopsie doopsie");
        ASSERT(psID.IsValid(), "oopsie doopsie");

        m_VertexShader = factory.CreateVertexShader();
        m_PixelShader = factory.CreatePixelShader();

        ConstView<Asset::Shader> vShaderData = assetManager.GetShader(vsID);
        ConstView<Asset::Shader> pxShaderData = assetManager.GetShader(psID);

        ASSERT(vShaderData.NonNull(), "uh-oh spaghetti-o!");
        ASSERT(pxShaderData.NonNull(), "uh-oh spaghetti-o!");

        api.CompileShaderAs(
            vShaderData->RawData.data(),
            vShaderData->RawData.size(),
            m_VertexShader
        );

        api.CompileShaderAs(
            pxShaderData->RawData.data(),
            pxShaderData->RawData.size(),
            m_PixelShader
        );

        m_InputLayout = factory.CreateInputLayout();
        m_VertexBuffer = factory.CreateVertexBuffer(Platform::BufferPolicy::Default);

        constexpr uint32_t SemanticIndex = 0;
        constexpr uint32_t VBufferSlot = 0;

        Platform::InputElement elems[] = {
            Platform::InputElement(
                Platform::StringView("POSITION"),
                SemanticIndex,
                Platform::DataFormat::Float32x2,
                VBufferSlot
            )
        };

        #define C_ARRAY_ARGS(x) x, std::size(x)

        api.InitializeInputLayout(
            m_InputLayout,
            m_VertexShader,
            C_ARRAY_ARGS(elems)
        );

        float vertices[] = {
            -0.5f, 0.0f,
             0.0f, 0.5f,
             0.5f, 0.0f
        };

        constexpr auto VerticesStride = sizeof(float) * 2;

        api.SetBuffer(
            m_VertexBuffer,
            Reinterpret<Platform::Byte*>(vertices),
            sizeof(vertices),
            VerticesStride,
            0,
            0
        );

        api.InitializeShader(m_VertexShader);
        api.InitializeShader(m_PixelShader);
    }

    Editor::~Editor() noexcept
    {
    }

    void Editor::Run() noexcept
    {
        using namespace Engine;

        Platform::Native& platform = m_Engine.Platform();
        Platform::IWindow& window = platform.Window();
        Platform::IContext& context = platform.Context();
        Platform::IApi& api = context.Api();

        api.BindInputLayout(m_InputLayout);

        api.BindShader(m_VertexShader);
        api.BindShader(m_PixelShader);

        api.BindBuffer(m_VertexBuffer);

        while (window.IsRunning())
        {
            using Clock = std::chrono::steady_clock;

            auto startTime = Clock::now();

            m_Engine.Update();

            constexpr float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            
            if (window.IsVisible())
            {
                api.Clear(ClearColor);

                constexpr auto NumVertices = 3;
                constexpr auto VerticesStartLocation = 0;

                /* Everything has to be rebinded after a resize... */
                if (api.HasResized())
                {
                    api.BindInputLayout(m_InputLayout);

                    api.BindShader(m_VertexShader);
                    api.BindShader(m_PixelShader);

                    api.BindBuffer(m_VertexBuffer);
                }

                api.Draw(NumVertices, VerticesStartLocation);

                api.Present();
            }

            constexpr auto TargetFrameTime = 1000.0f / 60.0f;

            auto endTime = Clock::now();

            using Milliseconds = std::chrono::duration<float, std::milli>;

            float frameMillis = std::chrono::duration_cast<Milliseconds>(endTime - startTime).count();

            float remainingMillis = TargetFrameTime - frameMillis;

            std::this_thread::sleep_for(Milliseconds(remainingMillis));
        }
    }
}