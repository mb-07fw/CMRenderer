#include "PCH.hpp"
#include "Renderer.hpp"
#include "Component.hpp"

namespace CMEngine::Renderer
{
	Renderer::Renderer(ECS::ECS& ecs, AGraphics& graphics, Asset::AssetManager& assetManager) noexcept
		: m_ECS(ecs),
		  m_Graphics(graphics),
		  m_BatchRenderer(m_ECS, m_Graphics, assetManager)
	{
		m_CB_CameraProj = m_Graphics.CreateBuffer(GPUBufferType::Constant, GPUBufferFlag::Dynamic);
	}

	Renderer::~Renderer() noexcept
	{
	}

	void Renderer::StartFrame(const Color4& clearColor) noexcept
	{
		m_Graphics.Clear(clearColor);
	}

	void Renderer::EndFrame() noexcept
	{
		m_Graphics.Present();
	}

	void Renderer::SetCamera(const CameraComponent& camera) noexcept
	{
		m_Graphics.SetBuffer(m_CB_CameraProj, &camera.Matrices, sizeof(camera.Matrices));
		m_Graphics.BindConstantBufferVS(m_CB_CameraProj, S_CB_CameraProj_Register);
	}

	void Renderer::Flush() noexcept
	{
		m_BatchRenderer.Flush();
	}
}