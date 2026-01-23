#include "Backend/Win32/D3D/_11/ApiResources_D3D11.hpp"
#include "Common/Cast.hpp"

namespace Platform::Backend::Win32::D3D::_11
{
    void InputLayout::Create(
        const ComPtr<::ID3DBlob>& pVSBytecodeWithInputSignature,
        const ComPtr<::ID3D11Device>& pDevice,
        std::span<const InputElement> elems
    ) noexcept
    {
		PLATFORM_FAILURE_IF(
			!pVSBytecodeWithInputSignature,
			"(InputLayout) Provided bytecode is nullptr."
		);

		PLATFORM_FAILURE_IF(
			!pDevice,
			"(InputLayout) Provided device is nullptr."
		);

		m_Elems.clear();
		m_Descs.clear();

        m_Elems.assign(elems.begin(), elems.end());

        TranslateElements(m_Elems, m_Descs);

		::HRESULT hr = pDevice->CreateInputLayout(
			m_Descs.data(),
			m_Descs.size(),
			pVSBytecodeWithInputSignature->GetBufferPointer(),
			pVSBytecodeWithInputSignature->GetBufferSize(),
			&mP_InputLayout
		);

		PLATFORM_FAILURE_IF_V(
			FAILED(hr),
			"(InputLayout) Failed to create interface. Error: {}",
			hr
		);
    }

    void InputLayout::TranslateElements(
        std::span<const InputElement> elems,
        std::vector<::D3D11_INPUT_ELEMENT_DESC>& outDescs
    ) noexcept
    {
		outDescs.reserve(m_Elems.size());

		uint32_t instanceByteOffset = 0;
		uint32_t vertexByteOffset = 0;

		for (InputElement& element : m_Elems)
		{
			uint32_t& currentByteOffset = element.InputClass == InputClass::PerInstance ?
				instanceByteOffset : vertexByteOffset;

			D3D11_INPUT_ELEMENT_DESC desc = {};
			DXGI_FORMAT dxgiFormat = DataFormatToDXGI(element.Format);

			desc.SemanticName = element.Name.Data();
			desc.SemanticIndex = element.Index;
			desc.Format = dxgiFormat;
			desc.InputSlot = element.InputSlot;
			desc.InputSlotClass = InputClassToD3D11(element.InputClass);
			desc.InstanceDataStepRate = element.InstanceStepRate;
			desc.AlignedByteOffset = element.AlignedByteOffset;

			if (!IsMatrixDataFormat(element.Format))
			{
				currentByteOffset += (uint32_t)BytesOfDataFormat(dxgiFormat);
				outDescs.emplace_back(desc);
				continue;
			}

			size_t rowBytes = BytesOfDataFormat(dxgiFormat);
			for (size_t row = 0; row < RowsOfMatrixDataFormat(element.Format); ++row)
			{
				desc.AlignedByteOffset = currentByteOffset;
				desc.SemanticIndex = (uint32_t)row;

				currentByteOffset += (uint32_t)rowBytes;
				outDescs.emplace_back(desc);
			}
		}
    }

	void InputLayout::Bind(const ComPtr<::ID3D11DeviceContext>& pContext) const noexcept
	{
		PLATFORM_FAILURE_IF(!pContext, "(Shader) Provided context is nullptr.");
		PLATFORM_FAILURE_IF(!mP_InputLayout, "(Shader) InputLayout interface is nullptr.");

		pContext->IASetInputLayout(mP_InputLayout.Get());
	}

	VertexBuffer::VertexBuffer(BufferPolicy policy) noexcept
		: Parent(policy)
	{
	}

	void VertexBuffer::Set(
		const ComPtr<::ID3D11Device>& pDevice,
		const ComPtr<::ID3D11DeviceContext>& pContext,
		const std::span<const Byte>& data,
		uint32_t strideBytes,
		uint32_t offsetBytes,
		uint32_t registerSlot
	) noexcept
	{
		size_t sizeBytes = data.size_bytes();
		uint32_t startPos = offsetBytes;
		uint32_t numElems = Cast<uint32_t>(sizeBytes / strideBytes);
		uint32_t endPos   = offsetBytes + (strideBytes * numElems);

		PLATFORM_FAILURE_IF_V(
			endPos > sizeBytes,
			"(VertexBuffer) Invalid buffer config: Parameters are set to read beyond "
			"the scope of the buffer, which is probably not desired. Buffer Size: `{}` "
			"Offset: `{}`, Elements: `{}`, End Pos (offsetBytes + (numElems * strideBytes): `{}`",
			sizeBytes, offsetBytes, numElems, endPos
		);

		PLATFORM_FAILURE_IF_V(
			registerSlot >= S_NumRegisterSlots,
			"(VertexBuffer) Invalid register slot. Provided Slot: `{}`, Range: [0, {} - 1]",
			registerSlot, S_NumRegisterSlots
		);

		m_StrideBytes = strideBytes;
		m_OffsetBytes = offsetBytes;
		m_RegisterSlot = registerSlot;

		Parent::Set(pDevice, pContext, data);
	}

	void VertexBuffer::Bind(const ComPtr<::ID3D11DeviceContext>& pContext) noexcept
	{
		PLATFORM_FAILURE_IF(
			pContext == nullptr,
			"(VertexBuffer_D3D11) Provided device context is nullptr."
		);

		PLATFORM_FAILURE_IF(
			mP_Buffer == nullptr,
			"(VertexBuffer_D3D11) Buffer interface is nullptr."
		);

		pContext->IASetVertexBuffers(
			m_RegisterSlot,
			1,
			mP_Buffer.GetAddressOf(),
			&m_StrideBytes,
			&m_OffsetBytes
		);
	}

	IndexBuffer::IndexBuffer(BufferPolicy policy) noexcept
		: Parent(policy)
	{
	}

	void IndexBuffer::Bind(const ComPtr<::ID3D11DeviceContext>& pContext) noexcept
	{
		PLATFORM_FAILURE_IF(
			pContext == nullptr,
			"(IndexBuffer_D3D11) Provided device context is nullptr."
		);

		PLATFORM_FAILURE_IF(
			mP_Buffer == nullptr,
			"(IndexBuffer_D3D11) Buffer interface is nullptr."
		);

		pContext->IASetIndexBuffer(
			mP_Buffer.Get(),
			m_IndexFormat,
			m_IndexStartOffset
		);
	}

	ConstantBuffer::ConstantBuffer(BufferPolicy policy) noexcept
		: Parent(policy)
	{
	}

	void ConstantBuffer::Bind(const ComPtr<::ID3D11DeviceContext>& pContext) noexcept
	{
		PLATFORM_FAILURE_IF(
			pContext == nullptr,
			"(ConstantBuffer_D3D11) Provided device context is nullptr."
		);

		PLATFORM_FAILURE_IF(
			mP_Buffer == nullptr,
			"(ConstantBuffer_D3D11) Buffer interface is nullptr."
		);

		constexpr uint32_t NumBuffers = 1;

		switch (m_BindStage)
		{
		case CBBindStage::Vertex:
			pContext->VSSetConstantBuffers(m_RegisterSlot, NumBuffers, mP_Buffer.GetAddressOf());
			break;
		case CBBindStage::Pixel:
			pContext->PSSetConstantBuffers(m_RegisterSlot, NumBuffers, mP_Buffer.GetAddressOf());
			break;
		case CBBindStage::Invalid: [[fallthrough]];
		default:
		{
			uint32_t bindStageUint32 = Cast<uint32_t>(m_BindStage);
			PLATFORM_FAILURE_V("(ConstantBuffer_D3D11) Invalid bind stage: {}", bindStageUint32);
		}
		}
	}
}