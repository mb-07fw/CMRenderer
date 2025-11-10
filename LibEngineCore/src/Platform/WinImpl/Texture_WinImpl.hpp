#pragma once

#include "Platform/Core/ITexture.hpp"
#include "IDXUploadable_WinImpl.hpp"

namespace CMEngine::Platform::WinImpl
{
	class Texture : public ITexture, public IDXUploadable
	{
	public:
		Texture() noexcept;
		~Texture() = default;

		void Create(
			std::span<const std::byte> data,
			const ComPtr<ID3D11Device>& pDevice
		) noexcept;

		void SetResourceSlot(uint32_t slot) noexcept;
		void SetSamplerSlot(uint32_t slot) noexcept;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;
	private:
		ComPtr<ID3D11Resource> mP_Texture;
		ComPtr<ID3D11ShaderResourceView> mP_TextureView;
		/* TODO: Move to Renderer, as this should be a renderer-specific resource, shared across many textures. */
		ComPtr<ID3D11SamplerState> mP_Sampler;
		uint32_t m_ResourceSlot = 0;
		uint32_t m_SamplerSlot = 0;
	};
}