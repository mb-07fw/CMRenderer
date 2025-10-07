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

		virtual [[nodiscard]] bool IsCreated() const noexcept = 0;
		virtual operator bool() const noexcept = 0;

		virtual [[nodiscard]] bool HasFlag(GPUBufferFlag flag) const noexcept = 0;

		inline static constexpr [[nodiscard]] D3D11_USAGE FlagsToUsage(GPUBufferFlag flags) noexcept;
		inline static constexpr [[nodiscard]] UINT FlagsToCPUAccess(GPUBufferFlag flags) noexcept;
	protected:
		void VerifyFlags(GPUBufferFlag& outFlags) noexcept;
	};

	class GPUBufferBasic : public IGPUBuffer
	{
	public:
		GPUBufferBasic(
			UINT bindFlags,
			UINT byteWidth = 0,
			GPUBufferFlag flags = GPUBufferFlag::Default,
			UINT miscFlags = 0,
			UINT structureByteStride = 0
		) noexcept;

		virtual ~GPUBufferBasic() = default;

		template <typename Ty>
		inline void Create(std::span<const Ty> data, const ComPtr<ID3D11Device>& pDevice) noexcept;
		virtual void Create(const void* pData, size_t numBytes, const ComPtr<ID3D11Device>& pDevice) noexcept override;
		virtual void Release() noexcept override;

		inline virtual [[nodiscard]] bool IsCreated() const noexcept override { return mP_Buffer.Get() != nullptr; }
		inline virtual operator bool() const noexcept override { return IsCreated(); }

		inline constexpr virtual [[nodiscard]] bool HasFlag(GPUBufferFlag flag) const noexcept override { return FlagUnderlying(m_Flags & flag); }
	protected:
		GPUBufferFlag m_Flags = GPUBufferFlag::Unspecified;
		CD3D11_BUFFER_DESC m_Desc = {};
		ComPtr<ID3D11Buffer> mP_Buffer;
	};

	class VertexBuffer : public GPUBufferBasic
	{
	public:
		VertexBuffer(
			UINT vertexByteStride,
			UINT vertexStartOffset = 0,
			UINT registerSlot = 0
		) noexcept;

		~VertexBuffer() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;

		/* Use these setters to override previously set properties. Overriden properties are NOT applied 
		 *   to already created buffers, they should instead be released and re-created. */
		inline void SetVertexStride(UINT vertexByteStride) noexcept { m_VertexByteStride = vertexByteStride; }
		inline void SetVertexOffset(UINT vertexStartOffset) noexcept { m_VertexStartOffset = vertexStartOffset; }
		inline void SetRegisterSlot(UINT registerSlot) noexcept { m_RegisterSlot = registerSlot; }
	private:
		UINT m_VertexByteStride = 0;
		UINT m_VertexStartOffset = 0;
		UINT m_RegisterSlot = 0;
	};

	class IndexBuffer : public GPUBufferBasic
	{
	public:
		IndexBuffer(
			DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT,
			UINT indexStartOffset = 0
		) noexcept;

		~IndexBuffer() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;

		/* Use these setters to override previously set properties. Overriden properties are NOT applied
		 *   to already created buffers, they should instead be released and re-created. */
		inline void SetFormat(DXGI_FORMAT indexFormat) noexcept { m_IndexFormat = indexFormat; }
		inline void SetOffset(UINT indexStartOffset) noexcept { m_IndexStartOffset = indexStartOffset; }
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
		ConstantBuffer(
			ConstantBufferType type = ConstantBufferType::INVALID,
			GPUBufferFlag flags = GPUBufferFlag::Default,
			UINT registerSlot = 0
		) noexcept;

		~ConstantBuffer() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept override;

		inline void SetType(ConstantBufferType type) noexcept { m_Type = type; }
	private:
		void Bind(const ComPtr<ID3D11DeviceContext>& pContext, ID3D11Buffer* pBuffer) const noexcept;
	private:
		static constexpr UINT S_TOTAL_REGISTERS = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		ConstantBufferType m_Type = ConstantBufferType::INVALID;
		UINT m_RegisterSlot = 0;
	};

	/* BufferArrayTyped is an array of homogenous buffer types, meaning it is not meant to be used polymorphically. */
	template <typename Ty>
		requires std::is_base_of_v<GPUBufferBasic, Ty> /* Ty is a derived type of GPUBufferBasic. */
	class BufferArrayTyped
	{
	public:
		inline BufferArrayTyped(size_t capacity) noexcept;
		~BufferArrayTyped() = default;

		template <typename... Args>
		inline Ty& EmplaceBack(Args&&... args) noexcept;

		inline void Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept;

		inline Ty& operator[](size_t index) noexcept { return m_Buffers[index]; }
		inline const Ty& operator[](size_t index) const noexcept { return m_Buffers[index]; }

		inline std::vector<Ty>& Array() noexcept { return m_Buffers; }
	private:
		static constexpr bool S_IS_VB_ARRAY = std::is_same_v<VertexBuffer, Ty>;
		static constexpr bool S_IS_IB_ARRAY = std::is_same_v<IndexBuffer, Ty>;
		static constexpr bool S_IS_CB_ARRAY = std::is_same_v<ConstantBuffer, Ty>;
		std::vector<Ty> m_Buffers;
	};

	using VertexBufferArray = BufferArrayTyped<VertexBuffer>;

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
	inline void GPUBufferBasic::Create(std::span<const Ty> data, const ComPtr<ID3D11Device>& pDevice) noexcept
	{
		Create(data.data(), data.size_bytes(), pDevice);
	}

	template <typename Ty>
		requires std::is_base_of_v<GPUBufferBasic, Ty>
	inline BufferArrayTyped<Ty>::BufferArrayTyped(size_t capacity) noexcept
	{
		m_Buffers.reserve(capacity);
	}

	template <typename Ty>
		requires std::is_base_of_v<GPUBufferBasic, Ty>
	template <typename... Args>
	inline Ty& BufferArrayTyped<Ty>::EmplaceBack(Args&&... args) noexcept
	{
		return m_Buffers.emplace_back(std::forward<Args>(args)...);
	}

	template <typename Ty>
		requires std::is_base_of_v<GPUBufferBasic, Ty>
	inline void BufferArrayTyped<Ty>::Upload(const ComPtr<ID3D11DeviceContext>& pContext) const noexcept
	{
		for (const auto& buffer : m_Buffers)
			buffer.Upload(pContext);
	}
}