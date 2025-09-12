#include "PCH.hpp"
#include "Platform/WinImpl/GPUBuffer_WinImpl.hpp"
#include "Macros.hpp"

namespace CMEngine::Platform::WinImpl
{
	IGPUBuffer::IGPUBuffer(
		UINT bindFlags,
		UINT byteWidth,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: m_Desc(
			byteWidth,
			bindFlags,
			usage,
			cpuAccessFlags,
			miscFlags,
			structureByteStride
		)
	{
	}

	void IGPUBuffer::Create(const void* pData, size_t numBytes, const ComPtr<ID3D11Device>& pDevice) noexcept
	{
		CM_ENGINE_ASSERT(pData != nullptr);
		CM_ENGINE_ASSERT(numBytes != 0);
		CM_ENGINE_ASSERT(pDevice.Get() != nullptr);
		CM_ENGINE_ASSERT(!IsCreated());

		m_Desc.ByteWidth = static_cast<UINT>(numBytes);

		D3D11_SUBRESOURCE_DATA subData = {};
		subData.pSysMem = pData;

		HRESULT hr = pDevice->CreateBuffer(&m_Desc, &subData, &mP_Buffer);

		CM_ENGINE_ASSERT(!FAILED(hr));
	}

	void IGPUBuffer::Release() noexcept
	{
		mP_Buffer.Reset();

		m_Desc.ByteWidth = 0;
		m_Desc.StructureByteStride = 0;
	}

	VertexBuffer::VertexBuffer(
		UINT vertexByteStride,
		UINT vertexStartOffset,
		UINT registerSlot
	) noexcept
		: IGPUBuffer(D3D11_BIND_VERTEX_BUFFER),
		  m_VertexByteStride(vertexByteStride),
		  m_RegisterSlot(registerSlot),
		  m_VertexStartOffset(vertexStartOffset)
	{
	}

	void VertexBuffer::Upload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		CM_ENGINE_ASSERT(IsCreated());

		constexpr UINT NumVertexBuffers = 1;

		pContext->IASetVertexBuffers(
			m_RegisterSlot,
			NumVertexBuffers,
			mP_Buffer.GetAddressOf(),
			&m_VertexByteStride,
			&m_VertexStartOffset
		);
	}

	void VertexBuffer::ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		pContext->IASetVertexBuffers(
			m_RegisterSlot,
			0,
			nullptr,
			0, 
			0
		);
	}

	IndexBuffer::IndexBuffer(DXGI_FORMAT indexFormat, UINT indexStartOffset) noexcept
		: IGPUBuffer(D3D11_BIND_INDEX_BUFFER),
		  m_IndexFormat(indexFormat),
		  m_IndexStartOffset(indexStartOffset)
	{
	}

	void IndexBuffer::Upload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		CM_ENGINE_ASSERT(IsCreated());

		pContext->IASetIndexBuffer(mP_Buffer.Get(), m_IndexFormat, m_IndexStartOffset);
	}

	void IndexBuffer::ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	}

	ConstantBuffer::ConstantBuffer(ConstantBufferType type, UINT registerSlot) noexcept
		: IGPUBuffer(D3D11_BIND_CONSTANT_BUFFER),
		  m_Type(type),
		  m_RegisterSlot(registerSlot)
	{
	}

	void ConstantBuffer::Upload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		Bind(pContext, mP_Buffer.Get());
	}

	void ConstantBuffer::ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept
	{
		Bind(pContext, nullptr);
	}

	void ConstantBuffer::Bind(const ComPtr<ID3D11DeviceContext>& pContext, ID3D11Buffer* pBuffer) noexcept
	{
		CM_ENGINE_ASSERT(m_RegisterSlot < S_TOTAL_REGISTERS);
		constexpr UINT NumConstantBuffers = 1;

		switch (m_Type)
		{
		case ConstantBufferType::VS:
			pContext->VSSetConstantBuffers(m_RegisterSlot, NumConstantBuffers, &pBuffer);
			break;
		case ConstantBufferType::PS:
			pContext->PSSetConstantBuffers(m_RegisterSlot, NumConstantBuffers, &pBuffer);
			break;
		case ConstantBufferType::INVALID: [[fallthrough]];
		default:
			CM_ENGINE_ASSERT(false);
		}
	}
}