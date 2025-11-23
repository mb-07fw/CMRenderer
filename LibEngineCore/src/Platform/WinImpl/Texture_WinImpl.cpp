#include "PCH.hpp"
#include "Texture_WinImpl.hpp"
#include "Macros.hpp"

namespace CMEngine::Platform::WinImpl
{
	Texture::Texture() noexcept
	{
	}

	void Texture::Create(
		std::span<const std::byte> data,
		const ComPtr<ID3D11Device>& pDevice
	) noexcept
	{
		HRESULT hr = DirectX::CreateWICTextureFromMemory(
			pDevice.Get(),
			data.data(),
			data.size(),
			&mP_Texture,
			&mP_TextureView
		);

		CM_ENGINE_ASSERT(!FAILED(hr));

		/* Used if D3D11_TEXTURE_ADDRESS_BORDER is provided for AddressU, AddressV, or AddressW. */
		constexpr FLOAT BorderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		CD3D11_SAMPLER_DESC sampDesc(
			D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			D3D11_TEXTURE_ADDRESS_WRAP, /* (texture tiling) */
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			0.0f, /* mip LOS bias */
			0, /* max anistropy */
			D3D11_COMPARISON_NEVER, /* comparison func */
			BorderColor,
			0.0f, /* min LOD */
			D3D11_FLOAT32_MAX /* max LOD, no upper limit on LOD */
		);

		CM_ENGINE_ASSERT(pDevice->CreateSamplerState(&sampDesc, mP_Sampler.GetAddressOf()) == S_OK);
	}

	void Texture::SetResourceSlot(uint32_t slot) noexcept
	{
		CM_ENGINE_ASSERT(m_ResourceSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		m_ResourceSlot = slot;
	}

	void Texture::SetSamplerSlot(uint32_t slot) noexcept
	{
		CM_ENGINE_ASSERT(m_SamplerSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
		m_SamplerSlot = slot;
	}

	void Texture::Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
	{
		constexpr uint32_t NumViews = 1;
		constexpr uint32_t NumSamplers = 1;

		pContext->PSSetShaderResources(m_ResourceSlot, NumViews, mP_TextureView.GetAddressOf());
		pContext->PSSetSamplers(m_SamplerSlot, NumSamplers, mP_Sampler.GetAddressOf());
	}

	void Texture::ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
	{
		constexpr uint32_t NumViews = 1;
		constexpr uint32_t NumSamplers = 1;

		pContext->PSSetShaderResources(m_ResourceSlot, NumViews, nullptr);
		pContext->PSSetSamplers(m_SamplerSlot, NumSamplers, nullptr);
	}
}