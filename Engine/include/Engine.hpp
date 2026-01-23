#pragma once

#include "Platform.hpp"
#include "Ref.hpp"
#include "Asset/AssetManager.hpp"

namespace Engine
{
    class Engine
    {
    public:
        Engine() noexcept;
        ~Engine() noexcept;

        void Update() noexcept;

        inline [[nodiscard]] Platform::Native& Platform() noexcept { return m_NativePlatform; }
        inline [[nodiscard]] Asset::AssetManager& AssetManager() noexcept { return m_AssetManager; }
    private:
        Platform::Native m_NativePlatform;
        Asset::AssetManager m_AssetManager;
    };
}