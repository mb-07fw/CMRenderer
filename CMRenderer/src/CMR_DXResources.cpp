#include "CMR_PCH.hpp"
#include "CMR_DXResources.hpp"

namespace CMRenderer::CMDirectX
{
	DXBufferMetaData::DXBufferMetaData(
		UINT bindFlags,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Desc(0u, bindFlags, usage, cpuAccessFlags, miscFlags, structureByteStride)
	{
	}

	[[nodiscard]] bool DXBufferMetaData::IsValid() const noexcept
	{
		if (Desc.ByteWidth == 0u ||
			Desc.BindFlags == 0u ||
			SubresourceData.pSysMem == nullptr)
			return false;

		/* TODO: Check for valid combinations between bind flags, usages, and cpu access flags. */

		return true;
	}

	DXBuffer::DXBuffer(DXBufferType type, DXBufferMetaData metaData) noexcept
		: m_Type(type),
		m_MetaData(metaData)
	{
		CM_ASSERT(type != DXBufferType::INVALID);
	}

	[[nodiscard]] HRESULT DXBuffer::Create(Components::DXDevice& deviceRef) noexcept
	{
		CM_ASSERT(!m_Created);
		CM_ASSERT(m_MetaData.IsValid());

		HRESULT hResult = deviceRef->CreateBuffer(m_MetaData.DescPtr(), m_MetaData.SubresourceDataPtr(), BufferAddress());

		if (!FAILED(hResult))
		{
			m_Created = true;
			m_Released = false;
		}

		return hResult;
	}

	void DXBuffer::Release() noexcept
	{
		CM_ASSERT(m_Created);
		CM_ASSERT(Buffer() != nullptr);

		mP_Buffer.Reset();

		m_Created = false;
		m_Released = true;
	}

	void DXBuffer::ReleaseIfCreated() noexcept
	{
		if (!m_Created)
			return;

		Release();
	}

	void DXBuffer::SetDesc(
		UINT bindFlags,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
	{
		ReleaseIfCreated();

		m_MetaData.Desc.BindFlags = bindFlags;
		m_MetaData.Desc.Usage = usage;
		m_MetaData.Desc.CPUAccessFlags = cpuAccessFlags;
		m_MetaData.Desc.MiscFlags = miscFlags;
		m_MetaData.Desc.StructureByteStride = structureByteStride;
	}

	DXVertexBuffer::DXVertexBuffer(
		UINT vertexStride,
		UINT registerSlot,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: DXBuffer(
			DXBufferType::VERTEX,
			DXBufferMetaData(
				D3D11_BIND_VERTEX_BUFFER,
				usage,
				cpuAccessFlags,
				miscFlags,
				structureByteStride
			)
		  ),
		  m_Stride(vertexStride),
		  m_Register(registerSlot)
	{
		CM_ASSERT(vertexStride != 0);
		CM_ASSERT(IsValidRegister(registerSlot));
	}

	void DXVertexBuffer::Bind(Components::DXDevice& deviceRef, UINT offset) noexcept
	{
		CM_ASSERT(Buffer() != nullptr);
		CM_ASSERT(IsCreated());

		deviceRef.ContextRaw()->IASetVertexBuffers(m_Register, 1u, BufferAddress(), &m_Stride, &offset);
	}

	void DXVertexBuffer::SetStride(UINT vertexStride) noexcept
	{
		CM_ASSERT(vertexStride != 0);

		ReleaseIfCreated();

		m_Stride = vertexStride;
	}

	DXIndexBuffer::DXIndexBuffer(
		DXGI_FORMAT format,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: DXBuffer(
			DXBufferType::INDEX,
			DXBufferMetaData(
				D3D11_BIND_INDEX_BUFFER,
				usage,
				cpuAccessFlags,
				miscFlags,
				structureByteStride
			)
		  ),
		  m_Format(format)
	{
		CM_ASSERT(format != DXGI_FORMAT_UNKNOWN);
	}

	void DXIndexBuffer::Bind(Components::DXDevice& deviceRef, UINT offset) noexcept
	{
		CM_ASSERT(Buffer() != nullptr);
		CM_ASSERT(IsCreated());

		deviceRef.ContextRaw()->IASetIndexBuffer(Buffer(), m_Format, offset);
	}

	void DXIndexBuffer::SetFormat(DXGI_FORMAT format) noexcept
	{
		CM_ASSERT(format != DXGI_FORMAT_UNKNOWN);

		ReleaseIfCreated();

		m_Format = format;
	}

	DXConstantBuffer::DXConstantBuffer(
		UINT registerSlot,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: DXBuffer(
			DXBufferType::CONSTANT,
			DXBufferMetaData(
				D3D11_BIND_CONSTANT_BUFFER,
				usage,
				cpuAccessFlags,
				miscFlags,
				structureByteStride
			)
		  ),
		  m_Register(registerSlot)
	{
		CM_ASSERT(IsValidRegister(registerSlot));
	}

	void DXConstantBuffer::BindVS(Components::DXDevice& deviceRef, UINT offset) noexcept
	{
		CM_ASSERT(Buffer() != nullptr);
		CM_ASSERT(IsCreated());

		deviceRef.ContextRaw()->VSSetConstantBuffers(m_Register, 1u, BufferAddress());
	}

	void DXConstantBuffer::BindPS(Components::DXDevice& deviceRef, UINT offset) noexcept
	{
		CM_ASSERT(Buffer() != nullptr);
		CM_ASSERT(IsCreated());
	}

	void DXConstantBuffer::SetRegister(UINT registerSlot) noexcept
	{
		CM_ASSERT(IsValidRegister(registerSlot));

		ReleaseIfCreated();

		m_Register = registerSlot;
	}
}