#pragma once

#include "ECS/ECS.hpp"
#include "Asset/AssetManager.hpp"
#include "Platform.hpp"
#include "BatchRenderer.hpp"

#include <array>
#include <vector>
#include <span>

namespace CMEngine::Renderer
{
	class Renderer
	{
	public:
		Renderer(ECS::ECS& ecs, AGraphics& graphics, Asset::AssetManager& assetManager) noexcept;
		~Renderer() noexcept;
	public:
		void StartFrame(const Color4& clearColor) noexcept;
		void EndFrame() noexcept;

		void SetCamera(const CameraComponent& camera) noexcept;

		void Flush() noexcept;

		[[nodiscard]] bool ImGuiWindow(const std::string_view& label) noexcept;
		void ImGuiEndWindow() noexcept;
		void ImGuiText(const std::string_view& text) noexcept;

		inline [[nodiscard]] BatchRenderer& GetBatchRenderer() noexcept { return m_BatchRenderer; }
	private:
		static constexpr uint32_t S_CB_CameraProj_Register = 0;
		ECS::ECS& m_ECS;
		AGraphics& m_Graphics; /* TODO: Technically, the renderer should own the GPU context, but idc rn... */
		BatchRenderer m_BatchRenderer;
		Resource<IBuffer> m_CB_CameraProj;
	};
}