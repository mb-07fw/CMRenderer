#include "Core/PCH.hpp"
#include "DX/DX11/DX11_Resources.hpp"

namespace CMEngine::DX::DX11
{
	BufferMetaData::BufferMetaData(
		UINT bindFlags,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Desc(0u, bindFlags, usage, cpuAccessFlags, miscFlags, structureByteStride)
	{
	}

	[[nodiscard]] bool BufferMetaData::IsValid() const noexcept
	{
		if (Desc.ByteWidth == 0u ||
			Desc.BindFlags == 0u ||
			SubresourceData.pSysMem == nullptr)
			return false;

		/* TODO: Check for valid combinations between bind flags, usages, and cpu access flags. */

		return true;
	}

	Buffer::Buffer(BufferType type, BufferMetaData metaData) noexcept
		: m_Type(type),
		  m_MetaData(metaData)
	{
		CM_ASSERT(type != BufferType::INVALID);
	}

	[[nodiscard]] HRESULT Buffer::Create(Device& device) noexcept
	{
		CM_ASSERT(!m_Created);
		CM_ASSERT(m_MetaData.IsValid());

		HRESULT hResult = device->CreateBuffer(
			m_MetaData.DescPtr(),
			m_MetaData.SubresourceDataPtr(),
			GetAddress()
		);

		if (!FAILED(hResult))
		{
			m_Created = true;
			m_Released = false;
		}

		return hResult;
	}

	void Buffer::Release() noexcept
	{
		CM_ASSERT(m_Created);
		CM_ASSERT(GetRaw() != nullptr);

		mP_Buffer.Reset();

		m_Created = false;
		m_Released = true;
	}

	void Buffer::ReleaseIfCreated() noexcept
	{
		if (!m_Created)
			return;

		Release();
	}

	void Buffer::SetDesc(
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

	VertexBuffer::VertexBuffer(
		UINT vertexStride,
		UINT registerSlot,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Buffer(
			BufferType::VERTEX,
			BufferMetaData(
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

	void VertexBuffer::Bind(Device& device, UINT offset) noexcept
	{
		CM_ASSERT(GetRaw() != nullptr);
		CM_ASSERT(IsCreated());

		device.ContextRaw()->IASetVertexBuffers(m_Register, 1u, GetAddress(), &m_Stride, &offset);
	}

	void VertexBuffer::SetStride(UINT vertexStride) noexcept
	{
		CM_ASSERT(vertexStride != 0);

		ReleaseIfCreated();

		m_Stride = vertexStride;
	}

	IndexBuffer::IndexBuffer(
		DXGI_FORMAT format,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Buffer(
			BufferType::INDEX,
			BufferMetaData(
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

	void IndexBuffer::Bind(Device& device, UINT offset) noexcept
	{
		CM_ASSERT(GetRaw() != nullptr);
		CM_ASSERT(IsCreated());

		device.ContextRaw()->IASetIndexBuffer(
			GetRaw(),
			m_Format, 
			offset
		);
	}

	void IndexBuffer::SetFormat(DXGI_FORMAT format) noexcept
	{
		CM_ASSERT(format != DXGI_FORMAT_UNKNOWN);

		ReleaseIfCreated();

		m_Format = format;
	}

	ConstantBuffer::ConstantBuffer(
		UINT registerSlot,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Buffer(
			BufferType::CONSTANT,
			BufferMetaData(
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

	void ConstantBuffer::BindVS(Device& device) noexcept
	{
		CM_ASSERT(GetRaw() != nullptr);
		CM_ASSERT(IsCreated());

		device.ContextRaw()->VSSetConstantBuffers(
			m_Register,
			1, 
			GetAddress()
		);
	}

	void ConstantBuffer::BindPS(Device& device) noexcept
	{
		CM_ASSERT(GetRaw() != nullptr);
		CM_ASSERT(IsCreated());

		device.ContextRaw()->PSSetConstantBuffers(
			m_Register,
			1,
			GetAddress()
		);
	}

	void ConstantBuffer::SetRegister(UINT registerSlot) noexcept
	{
		CM_ASSERT(IsValidRegister(registerSlot));

		ReleaseIfCreated();

		m_Register = registerSlot;
	}
}