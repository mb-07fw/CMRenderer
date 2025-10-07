#pragma once

#include "Platform/Core/IUploadable.hpp"
#include "Platform/WinImpl/Types_WinImpl.hpp"

#include <cstdint>
#include <span>
#include <vector>
#include <type_traits>

#include <d3d11.h>

namespace CMEngine::Platform::WinImpl
{
	/* A pure virtual interface for an object that is bindable to the D3D11 pipeline. */
	class IDXUploadable : public IUploadable
	{
	public:
		IDXUploadable() = default;
		virtual ~IDXUploadable() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept = 0;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept = 0;
	};

	/* A pure virtual interface for a buffer that is bindable to the D3D11 pipeline.
	 * The main distinction between IGPUBuffer and ICPUBuffer is that IGPUBuffer's will never
	 *   store data, only marshal it to the GPU.
	 * 
	 * For an ICPUBuffer derivative to be uploaded to the GPU, it must first be converted to an IGPUBuffer derivative. */
	class IGPUBuffer : public IDXUploadable
	{
	public:
		IGPUBuffer() = default;
		virtual ~IGPUBuffer() = default;

		virtual void Create(const void* pData, size_t numBytes, const ComPtr<ID3D11Device>& pDevice) noexcept = 0;
		virtual void Release() noexcept = 0;

		virtual void Update(void* pData, size_t numBytes, const ComPtr<ID3D11DeviceContext>& pContext) noexcept = 0;

		virtual [[nodiscard]] bool IsCreated() const noexcept = 0;
		virtual operator bool() const noexcept = 0;

		virtual [[nodiscard]] bool HasFlag(GPUBufferFlag flag) const noexcept = 0;

		inline static constexpr [[nodiscard]] D3D11_BIND_FLAG TypeToBindFlags(GPUBufferType type) noexcept;
		inline static constexpr [[nodiscard]] D3D11_USAGE FlagsToUsage(GPUBufferFlag flags) noexcept;
		inline static constexpr [[nodiscard]] UINT FlagsToCPUAccess(GPUBufferFlag flags) noexcept;
	protected:
		void VerifyFlags(GPUBufferFlag& outFlags) noexcept;
	};

	class GPUBufferBasic : public IGPUBuffer
	{
	public:
		GPUBufferBasic(
			GPUBufferType type,
			GPUBufferFlag flags = GPUBufferFlag::Default,
			UINT byteWidth = 0,
			UINT miscFlags = 0,
			UINT structureByteStride = 0
		) noexcept;

		virtual ~GPUBufferBasic() = default;

		template <typename Ty>
		inline void Create(const std::span<Ty>& data, const ComPtr<ID3D11Device>& pDevice) noexcept;
		virtual void Create(const void* pData, size_t numBytes, const ComPtr<ID3D11Device>& pDevice) noexcept override;
		virtual void Release() noexcept override;

		virtual void Update(void* pData, size_t numBytes, const ComPtr<ID3D11DeviceContext>& pContext) noexcept override;

		inline virtual [[nodiscard]] bool IsCreated() const noexcept override { return mP_Buffer.Get() != nullptr; }
		inline virtual operator bool() const noexcept override { return IsCreated(); }

		inline constexpr virtual [[nodiscard]] bool HasFlag(GPUBufferFlag flag) const noexcept override { return FlagUnderlying(m_Flags & flag); }
	protected:
		GPUBufferType m_Type = GPUBufferType::Invalid;
		GPUBufferFlag m_Flags = GPUBufferFlag::Unspecified;
		CD3D11_BUFFER_DESC m_Desc = {};
		ComPtr<ID3D11Buffer> mP_Buffer;
	};

	class VertexBuffer : public GPUBufferBasic
	{
	public:
		VertexBuffer(GPUBufferFlag flags = GPUBufferFlag::Default) noexcept;
		~VertexBuffer() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;

		/* Use these setters to override previously set properties. Overriden properties are NOT applied 
		 *   to already created buffers, they should instead be released and re-created. */
		inline void SetStride(UINT strideBytes) noexcept { m_StrideBytes = strideBytes; }
		inline void SetOffset(UINT offsetBytes) noexcept { m_OffsetBytes = offsetBytes; }
		inline void SetRegister(UINT slot) noexcept { m_Register = slot; }

		inline UINT Stride() const noexcept { return m_StrideBytes; }
		inline UINT Offset() const noexcept { return m_OffsetBytes; }
		inline UINT Register() const noexcept { return m_Register; }
	private:
		UINT m_StrideBytes = 0;
		UINT m_OffsetBytes = 0;
		UINT m_Register = 0;
	};

	class IndexBuffer : public GPUBufferBasic
	{
	public:
		IndexBuffer(GPUBufferFlag flags = GPUBufferFlag::Default) noexcept;
		~IndexBuffer() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;

		/* Use these setters to override previously set properties. Overriden properties are NOT applied
		 *   to already created buffers, they should instead be released and re-created. */
		inline void SetFormat(DXGI_FORMAT indexFormat) noexcept { m_IndexFormat = indexFormat; }
		inline void SetOffset(UINT indexStartOffset) noexcept { m_IndexStartOffset = indexStartOffset; }

		inline [[nodiscard]] DXGI_FORMAT Format() const noexcept { return m_IndexFormat; }
		inline [[nodiscard]] UINT Offset() const noexcept { return m_IndexStartOffset; }
	private:
		DXGI_FORMAT m_IndexFormat = DXGI_FORMAT_UNKNOWN;
		UINT m_IndexStartOffset = 0;
	};

	enum class ConstantBufferType : int8_t
	{
		INVALID = -1,
		VS,
		PS
	};

	class ConstantBuffer : public GPUBufferBasic
	{
	public:
		ConstantBuffer(GPUBufferFlag flags = GPUBufferFlag::Default) noexcept;
		~ConstantBuffer() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;

		/* Use these setters to override previously set properties. Overriden properties are NOT applied
		 *   to already created buffers, they should instead be released and re-created. */
		inline void SetType(ConstantBufferType type) noexcept { m_Type = type; }
		inline void SetRegister(UINT slot) noexcept { m_RegisterSlot = slot; }
	private:
		void Bind(const ComPtr<ID3D11DeviceContext>& pContext, ID3D11Buffer* pBuffer) const noexcept;
	private:
		static constexpr UINT S_TOTAL_REGISTERS = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		ConstantBufferType m_Type = ConstantBufferType::INVALID;
		UINT m_RegisterSlot = 0;
	};

	inline constexpr [[nodiscard]] D3D11_BIND_FLAG IGPUBuffer::TypeToBindFlags(GPUBufferType type) noexcept
	{
		switch (type)
		{
		case GPUBufferType::Invalid: [[fallthrough]];
		default:
			return static_cast<D3D11_BIND_FLAG>(-1);
		case GPUBufferType::Vertex:
			return D3D11_BIND_VERTEX_BUFFER;
		case GPUBufferType::Index:
			return D3D11_BIND_INDEX_BUFFER;
		case GPUBufferType::Constant:
			return D3D11_BIND_CONSTANT_BUFFER;
		}
	}

	inline constexpr [[nodiscard]] D3D11_USAGE IGPUBuffer::FlagsToUsage(GPUBufferFlag flags) noexcept
	{
		UINT usage = 0;

		/* NOTE: Multiple basic usage flags are likely to result in buffer creation failure,
		 *		    but it is not this function's responsibility to respond accordingly. */
		if (FlagUnderlying(flags & GPUBufferFlag::Default))
			usage |= D3D11_USAGE_DEFAULT;
		if (FlagUnderlying(flags & GPUBufferFlag::Immutable))
			usage |= D3D11_USAGE_IMMUTABLE;
		if (FlagUnderlying(flags & GPUBufferFlag::Dynamic))
			usage |= D3D11_USAGE_DYNAMIC;
		if (FlagUnderlying(flags & GPUBufferFlag::Staging))
			usage |= D3D11_USAGE_STAGING;
		
		return static_cast<D3D11_USAGE>(usage);
	}

	inline constexpr [[nodiscard]] UINT IGPUBuffer::FlagsToCPUAccess(GPUBufferFlag flags) noexcept
	{
		UINT cpuFlags = 0;

		if (FlagUnderlying(flags & GPUBufferFlag::Read))
			cpuFlags |= D3D11_CPU_ACCESS_READ;
		if (FlagUnderlying(flags & GPUBufferFlag::Write))
			cpuFlags |= D3D11_CPU_ACCESS_WRITE;
		
		return cpuFlags;
	}

	template <typename Ty>
	inline void GPUBufferBasic::Create(const std::span<Ty>& data, const ComPtr<ID3D11Device>& pDevice) noexcept
	{
		Create(data.data(), data.size_bytes(), pDevice);
	}
}