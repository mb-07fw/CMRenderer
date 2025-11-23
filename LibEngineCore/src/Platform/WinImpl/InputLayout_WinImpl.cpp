#include "PCH.hpp"
#include "Platform/WinImpl/InputLayout_WinImpl.hpp"
#include "Macros.hpp"

namespace CMEngine::Platform::WinImpl
{
	InputLayout::InputLayout(std::span<const InputElement> elements) noexcept
		: InputLayoutBase(elements)
	{
		ConvertElements();
	}

	void InputLayout::Create(
		const ComPtr<ID3DBlob>& pVSBytecodeWithInputSignature,
		const ComPtr<ID3D11Device>& pDevice
	) noexcept
	{
		HRESULT hr = pDevice->CreateInputLayout(
			m_Descs.data(),
			(UINT)m_Descs.size(),
			pVSBytecodeWithInputSignature->GetBufferPointer(),
			pVSBytecodeWithInputSignature->GetBufferSize(),
			&mP_InputLayout
		);

		CM_ENGINE_ASSERT(!FAILED(hr));
	}

	void InputLayout::Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
	{
		CM_ENGINE_ASSERT(mP_InputLayout.Get() != nullptr);
		pContext->IASetInputLayout(mP_InputLayout.Get());
	}

	void InputLayout::ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
	{
		pContext->IASetInputLayout(nullptr);
	}

	/* Techincally, manually specifiying byte offsets isn't necessary due to
		 D3D11_APPEND_ALIGNED_ELEMENT, but whatever... */
	void InputLayout::ConvertElements() noexcept
	{
		m_Descs.reserve(m_Elements.size());

		uint32_t instanceByteOffset = 0;
		uint32_t vertexByteOffset = 0;

		for (InputElement& element : m_Elements)
		{
			uint32_t& currentByteOffset = element.InputClass == InputClass::PerInstance ?
				instanceByteOffset : vertexByteOffset;

			if (element.AlignedByteOffset == G_InputElement_InferByteOffset)
					element.AlignedByteOffset = currentByteOffset;

			D3D11_INPUT_ELEMENT_DESC desc = {};
			DXGI_FORMAT dxgiFormat = DataToDXGI(element.Format);

			desc.SemanticName = element.Name.data();
			desc.SemanticIndex = element.Index;
			desc.Format = dxgiFormat;
			desc.InputSlot = element.InputSlot;
			desc.InputSlotClass = InputClassToD3D11(element.InputClass);
			desc.InstanceDataStepRate = element.InstanceStepRate;
			desc.AlignedByteOffset = element.AlignedByteOffset;

			if (!IsMatrixFormat(element.Format))
			{
				currentByteOffset += (uint32_t)BytesOfFormat(dxgiFormat);
				m_Descs.emplace_back(desc);
				continue;
			}

			size_t rowBytes = BytesOfFormat(dxgiFormat);
			for (size_t row = 0; row < RowsOfMatrixFormat(element.Format); ++row)
			{
				desc.AlignedByteOffset = currentByteOffset;
				desc.SemanticIndex = (uint32_t)row;

				currentByteOffset += (uint32_t)rowBytes;
				m_Descs.emplace_back(desc);
			}
		}
	}
}