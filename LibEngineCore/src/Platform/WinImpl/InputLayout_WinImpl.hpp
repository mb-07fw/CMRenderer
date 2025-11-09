#pragma once

#include "Platform/Core/InputLayout.hpp"
#include "Platform/WinImpl/IDXUploadable_WinImpl.hpp"

#include <vector>
#include <d3d11.h>

namespace CMEngine::Platform::WinImpl
{
	/* shhhhhh, we don't talk about multiple inheritance around here... */
	class InputLayout : public InputLayoutBase, public IDXUploadable
	{
	public:
		InputLayout(std::span<const InputElement> elements) noexcept;
		~InputLayout() = default;
	public:
		void Create(
			const ComPtr<ID3DBlob>& pVSBytecodeWithInputSignature,
			const ComPtr<ID3D11Device>& pDevice
		) noexcept;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;

		inline [[nodiscard]] const std::vector<D3D11_INPUT_ELEMENT_DESC>& NativeElements() const noexcept { return m_Descs; }
	private:
		void ConvertElements() noexcept;
	private:
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_Descs;
		ComPtr<ID3D11InputLayout> mP_InputLayout;
	};
}