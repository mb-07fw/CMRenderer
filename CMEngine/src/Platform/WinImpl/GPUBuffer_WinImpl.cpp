#include "PCH.hpp"
#include "Platform/WinImpl/GPUBuffer_WinImpl.hpp"
#include "Macros.hpp"

namespace CMEngine::Platform::WinImpl
{
	void IGPUBuffer::VerifyFlags(GPUBufferFlag& outFlags) noexcept
	{
		/* If the basic no-read-write flags are set, there's no need to continue. */
		if (outFlags == GPUBufferFlag::Default ||
			outFlags == GPUBufferFlag::Immutable)
			return;

		bool containsBasicUsage = false;
		bool containsMultipleBasicUsage = false;

		if (FlagUnderlying(outFlags & GPUBufferFlag::Default))
		{
			containsBasicUsage = true;

			if (FlagUnderlying(outFlags & GPUBufferFlag::Read) ||
				FlagUnderlying(outFlags & GPUBufferFlag::Write))
				spdlog::warn(
					"(IGPUBuffer) internal warning: Provided 'Default' GPUBufferFlag contains Read/Write flags. "
					"This is invalid since a 'Default' GPUBuffer isn't permitted any CPU access for the best GPU-side "
					"memory optimization. If only CPU-side writing is required, the Dynamic_Write flag should be used, "
					"otherwise Staging_Read, Staging_Write, or Staging_ReadWrite are required."
				);
		}

		if (FlagUnderlying(outFlags & GPUBufferFlag::Immutable))
		{
			containsMultipleBasicUsage = containsBasicUsage;
			containsBasicUsage = true;

			if (FlagUnderlying(outFlags & GPUBufferFlag::Read) ||
				FlagUnderlying(outFlags & GPUBufferFlag::Write))
				spdlog::warn(
					"(IGPUBuffer) internal warning: Provided 'Immutable' GPUBufferFlag contains Read/Write flags. "
					"This is invalid since an 'Immutable' GPUBuffer isn't permitted any CPU access for the best GPU-side "
					"memory optimization. If only CPU-side writing is required, the Dynamic_Write flag should be used, "
					"otherwise Staging_Read, Staging_Write, or Staging_ReadWrite are required."
				);
		}

		if (FlagUnderlying(outFlags & GPUBufferFlag::Dynamic))
		{
			containsMultipleBasicUsage = containsBasicUsage;
			containsBasicUsage = true;

			if (FlagUnderlying(outFlags & GPUBufferFlag::Read))
				spdlog::warn(
					"(IGPUBuffer) internal warning: Provided 'Dynamic' GPUBufferFlag contains Read flag. "
					"This is fine, as it is possible to read data on the CPU from a 'Dynamic' buffer, but is usually bad practice "
					"due to 'Dynamic' being optimized for high-frequency CPU-Write operations, and will therefore incur an unneccesary performance toll "
					"compared to a 'Staging' GPUBuffer."
				);
			if (!FlagUnderlying(outFlags & GPUBufferFlag::Write))
			{
				outFlags |= GPUBufferFlag::Write;
			}
		}

		if (FlagUnderlying(outFlags & GPUBufferFlag::Staging))
		{
			containsMultipleBasicUsage = containsBasicUsage;

			if (!FlagUnderlying(outFlags & GPUBufferFlag::Read) &&
				!FlagUnderlying(outFlags & GPUBufferFlag::Write))
			{
				spdlog::warn("(IGPUBuffer) internal warning: Provided 'Staging' GPUBufferFlag doesn't specify CPU access. CPU-Read access is assumed.");
				outFlags |= GPUBufferFlag::Read;
			}
		}

		if (containsMultipleBasicUsage)
			spdlog::warn(
				"(IGPUBuffer) internal warning: Provided GPUBufferFlag contains multiple basic usage flags (Default, Immutable, etc). "
				"This is invalid since these flags represent different semantics in the graphics pipeline, "
				"and will most likely result in failure of buffer creation through the graphics API."
			);
	}

	GPUBufferBasic::GPUBufferBasic(
		UINT bindFlags,
		UINT byteWidth,
		GPUBufferFlag flags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: m_Flags(flags),
		  m_Desc(
			byteWidth,
			bindFlags,
			D3D11_USAGE_DEFAULT,
			0,
			miscFlags,
			structureByteStride
		)
	{
		VerifyFlags(flags);

		m_Desc.Usage = FlagsToUsage(flags);
		m_Desc.CPUAccessFlags = FlagsToCPUAccess(flags);
	}

	void GPUBufferBasic::Create(const void* pData, size_t numBytes, const ComPtr<ID3D11Device>& pDevice) noexcept
	{
		CM_ENGINE_ASSERT(pData != nullptr);
		CM_ENGINE_ASSERT(numBytes != 0);
		CM_ENGINE_ASSERT(pDevice.Get() != nullptr);

		mP_Buffer.Reset();

		m_Desc.ByteWidth = static_cast<UINT>(numBytes);

		D3D11_SUBRESOURCE_DATA subData = {};
		subData.pSysMem = pData;

		HRESULT hr = pDevice->CreateBuffer(&m_Desc, &subData, &mP_Buffer);

		CM_ENGINE_ASSERT(!FAILED(hr));
	}

	void GPUBufferBasic::Release() noexcept
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
		: GPUBufferBasic(D3D11_BIND_VERTEX_BUFFER),
		  m_VertexByteStride(vertexByteStride),
		  m_RegisterSlot(registerSlot),
		  m_VertexStartOffset(vertexStartOffset)
	{
	}

	void VertexBuffer::Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
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

	void VertexBuffer::ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
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
		: GPUBufferBasic(D3D11_BIND_INDEX_BUFFER),
		  m_IndexFormat(indexFormat),
		  m_IndexStartOffset(indexStartOffset)
	{
	}

	void IndexBuffer::Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
	{
		CM_ENGINE_ASSERT(IsCreated());

		pContext->IASetIndexBuffer(mP_Buffer.Get(), m_IndexFormat, m_IndexStartOffset);
	}

	void IndexBuffer::ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
	{
		pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	}

	ConstantBuffer::ConstantBuffer(
		ConstantBufferType type,
		GPUBufferFlag flags,
		UINT registerSlot
	) noexcept
		: GPUBufferBasic(D3D11_BIND_CONSTANT_BUFFER, 0, flags),
		  m_Type(type),
		  m_RegisterSlot(registerSlot)
	{
	}

	void ConstantBuffer::Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
	{
		Bind(pContext, mP_Buffer.Get());
	}

	void ConstantBuffer::ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
	{
		Bind(pContext, nullptr);
	}

	void ConstantBuffer::Bind(const ComPtr<ID3D11DeviceContext>& pContext, ID3D11Buffer* pBuffer) const noexcept
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