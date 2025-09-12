#pragma once

#include "Platform/WinImpl/Types_WinImpl.hpp"

#include <cstdint>
#include <span>

namespace CMEngine::Platform::WinImpl
{
	/* A pure virtual interface for an object that is bindable to the D3D11 pipeline. */
	class IUploadable
	{
	public:
		IUploadable() = default;
		virtual ~IUploadable() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept = 0;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept = 0;
	};

	/* A pure virtual representation of a buffer that is bindable to the D3D11 pipeline.
	 * The main distinction between IGPUBuffer and ICPUBuffer is that IGPUBuffer's will never
	 *   store data, only marshal it to the GPU.
	 * 
	 * For an ICPUBuffer derivative to be uploaded to the GPU, it must first be converted to an IGPUBuffer Derivative.
	 */
	class IGPUBuffer : public IUploadable
	{
	public:
		IGPUBuffer(
			UINT bindFlags,
			UINT byteWidth = 0,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0,
			UINT miscFlags = 0,
			UINT structureByteStride = 0
		) noexcept;

		virtual ~IGPUBuffer() = default;

		template <typename Ty>
		inline void Create(std::span<const Ty> data, const ComPtr<ID3D11Device>& pDevice) noexcept;
		void Create(const void* pData, size_t numBytes, const ComPtr<ID3D11Device>& pDevice) noexcept;
		void Release() noexcept;

		virtual [[nodiscard]] bool IsCreated() const noexcept { return mP_Buffer.Get() != nullptr; }
		virtual operator bool() const noexcept { return IsCreated(); }
	protected:
		CD3D11_BUFFER_DESC m_Desc = {};
		ComPtr<ID3D11Buffer> mP_Buffer;
	};

	class VertexBuffer : public IGPUBuffer
	{
	public:
		VertexBuffer(
			UINT vertexByteStride,
			UINT vertexStartOffset = 0,
			UINT registerSlot = 0
		) noexcept;

		~VertexBuffer() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept override;

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

	class IndexBuffer : public IGPUBuffer
	{
	public:
		IndexBuffer(
			DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT,
			UINT indexStartOffset = 0
		) noexcept;

		~IndexBuffer() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept override;

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

	class ConstantBuffer : public IGPUBuffer
	{
	public:
		ConstantBuffer(ConstantBufferType type, UINT registerSlot = 0) noexcept;

		~ConstantBuffer() = default;

		virtual void Upload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept override;
		virtual void ClearUpload(const ComPtr<ID3D11DeviceContext>& pContext) noexcept override;
	private:
		void Bind(const ComPtr<ID3D11DeviceContext>& pContext, ID3D11Buffer* pBuffer) noexcept;
	private:
		static constexpr UINT S_TOTAL_REGISTERS = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		ConstantBufferType m_Type = ConstantBufferType::INVALID;
		UINT m_RegisterSlot = 0;
	};

	template <typename Ty>
	inline void IGPUBuffer::Create(std::span<const Ty> data, const ComPtr<ID3D11Device>& pDevice) noexcept
	{
		Create(data.data(), data.size_bytes());
	}
}