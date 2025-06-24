#pragma once

#include "DirectX/CME_DXComponents.hpp"
#include "CMC_Macros.hpp"

#include <d3d11.h>
#include <wrl/client.h>

#include <span>
#include <vector>
#include <type_traits>

namespace CMEngine::DirectXAPI::DX11
{
	inline constexpr D3D11_BIND_FLAG CM_BUFFER_BIND_FLAG_INVALID = static_cast<D3D11_BIND_FLAG>(-1);
	inline constexpr D3D11_USAGE CM_BUFFER_USAGE_INVALID = static_cast<D3D11_USAGE>(-1);
	inline constexpr UINT CM_CONSTANT_BUFFER_REGISTER_SLOT_INVALID = static_cast<UINT>(-1);

	enum class DXBufferType : uint8_t
	{
		INVALID,
		VERTEX,
		INDEX,
		CONSTANT,
		STRUCTURED
	};

#pragma region Buffers
	struct DXBufferMetaData
	{
		DXBufferMetaData() = default;
		~DXBufferMetaData() = default;

		DXBufferMetaData(
			UINT bindFlags,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		template <typename DataTy>
			requires std::is_trivially_copyable_v<DataTy>
		inline DXBufferMetaData(
			std::span<DataTy> data,
			UINT bindFlags,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		[[nodiscard]] bool IsValid() const noexcept;

		inline [[nodiscard]] CD3D11_BUFFER_DESC* DescPtr() { return &Desc; }
		inline [[nodiscard]] D3D11_SUBRESOURCE_DATA* SubresourceDataPtr() { return &SubresourceData; }

		CD3D11_BUFFER_DESC Desc;
		D3D11_SUBRESOURCE_DATA SubresourceData = {};
	};

	class DXBuffer
	{
	public:
		DXBuffer(DXBufferType type, DXBufferMetaData metaData = DXBufferMetaData()) noexcept;
		DXBuffer() = default;
		virtual ~DXBuffer() = default;
	public:
		[[nodiscard]] HRESULT Create(DXDevice& device) noexcept;
		void Release() noexcept;
		void ReleaseIfCreated() noexcept;

		void SetDesc(
			UINT bindFlags,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		template <typename DataTy>
			requires std::is_trivially_copyable_v<DataTy>
		inline void SetData(std::span<DataTy> data) noexcept;

		inline [[nodiscard]] DXBufferType Type() const noexcept { return m_Type; }
		inline [[nodiscard]] const DXBufferMetaData& MetaData() const noexcept { return m_MetaData; }

		inline [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11Buffer> BufferComPtr() { return mP_Buffer; }
		inline [[nodiscard]] ID3D11Buffer* Buffer() { return mP_Buffer.Get(); }
		inline [[nodiscard]] ID3D11Buffer** BufferAddress() { return mP_Buffer.GetAddressOf(); }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	protected:
		DXBufferType m_Type = DXBufferType::INVALID;
		DXBufferMetaData m_MetaData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mP_Buffer;
		bool m_Created = false;
		bool m_Released = false;
	};

	class DXVertexBuffer : public DXBuffer
	{
	public:
		DXVertexBuffer(
			UINT vertexStride,
			UINT registerSlot = 0u,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		template <typename DataTy>
			requires std::is_trivially_copyable_v<DataTy>
		inline DXVertexBuffer(
			UINT vertexStride,
			std::span<DataTy> data,
			UINT registerSlot = 0u,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		DXVertexBuffer() = default;
		~DXVertexBuffer() = default;
	public:
		inline static constexpr [[nodiscard]] bool IsValidRegister(UINT slot) noexcept;
		inline static constexpr [[nodiscard]] UINT MaxRegisterSlot() noexcept { return D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1; }

		void Bind(DXDevice& device, UINT offset = 0u) noexcept;
		void SetStride(UINT vertexStride) noexcept;

		inline [[nodiscard]] UINT Stride() const noexcept { return m_Stride; }
	private:
		UINT m_Stride = 0u;
		UINT m_Register = 0u;
	};

	class DXIndexBuffer : public DXBuffer
	{
	public:
		DXIndexBuffer(
			DXGI_FORMAT format,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		template <typename DataTy>
			requires std::is_trivially_copyable_v<DataTy>
		inline DXIndexBuffer(
			DXGI_FORMAT format,
			std::span<DataTy> data,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		DXIndexBuffer() = default;
		~DXIndexBuffer() = default;
	public:
		void Bind(DXDevice& device, UINT offset = 0u) noexcept;
		void SetFormat(DXGI_FORMAT format) noexcept;
		
		inline [[nodiscard]] DXGI_FORMAT Format() const noexcept { return m_Format; }
	private:
		DXGI_FORMAT m_Format = DXGI_FORMAT_UNKNOWN;
	};

	class DXConstantBuffer : public DXBuffer
	{
	public:
		DXConstantBuffer(
			UINT registerSlot,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		template <typename DataTy>
			requires std::is_trivially_copyable_v<DataTy>
		inline DXConstantBuffer(
			UINT registerSlot,
			std::span<DataTy> data,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		DXConstantBuffer() = default;
		~DXConstantBuffer() = default;
	public:
		inline static constexpr [[nodiscard]] bool IsValidRegister(UINT slot) noexcept;

		void BindVS(DXDevice& device, UINT offset = 0u) noexcept;
		void BindPS(DXDevice& device, UINT offset = 0u) noexcept;
		void SetRegister(UINT registerSlot) noexcept;

		inline [[nodiscard]] UINT Register() const noexcept { return m_Register; }
	private:
		UINT m_Register = 0u;
	};

	template <typename DataTy>
		requires std::is_trivially_copyable_v<DataTy>
	inline DXBufferMetaData::DXBufferMetaData(
		std::span<DataTy> data,
		UINT bindFlags,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Desc(static_cast<UINT>(data.size_bytes()), bindFlags, usage, cpuAccessFlags, miscFlags, structureByteStride)
	{
		SubresourceData.pSysMem = data.data();
	}

	template <typename DataTy>
		requires std::is_trivially_copyable_v<DataTy>
	inline void DXBuffer::SetData(std::span<DataTy> data) noexcept
	{
		ReleaseIfCreated();

		CM_ASSERT(data.data() != nullptr && data.size_bytes() != 0u);

		m_MetaData.SubresourceData.pSysMem = data.data();
		m_MetaData.Desc.ByteWidth = static_cast<UINT>(data.size_bytes());
	}

	template <typename DataTy>
		requires std::is_trivially_copyable_v<DataTy>
	inline DXVertexBuffer::DXVertexBuffer(
		UINT vertexStride,
		std::span<DataTy> data,
		UINT registerSlot,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: DXBuffer(
			DXBufferType::VERTEX,
			DXBufferMetaData(
				data,
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

	inline constexpr [[nodiscard]] bool DXVertexBuffer::IsValidRegister(UINT slot) noexcept
	{
		/* The maximum of 16 or 32 input slots (ranges from 0 to D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1)
		 * are available; the maximum number of input slots depends on the feature level.
		 */
		return slot < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
	}

	template <typename DataTy>
		requires std::is_trivially_copyable_v<DataTy>
	inline DXIndexBuffer::DXIndexBuffer(
		DXGI_FORMAT format,
		std::span<DataTy> data,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: DXBuffer(
			DXBufferType::INDEX,
			DXBufferMetaData( 
				data,
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

	template <typename DataTy>
		requires std::is_trivially_copyable_v<DataTy>
	inline DXConstantBuffer::DXConstantBuffer(
		UINT registerSlot,
		std::span<DataTy> data,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: DXBuffer(
			DXBufferType::CONSTANT,
			DXBufferMetaData(
				data,
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

	inline constexpr [[nodiscard]] bool DXConstantBuffer::IsValidRegister(UINT slot) noexcept
	{
		/* Index into the device's zero-based array to begin setting constant buffers to
		 * (ranges from 0 to D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1).
		 */
		return slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	}

	/* Wraps a buffer type, and stores it's data locally to ensure proper lifetime semantics. */
	template <typename DataTy, typename DXBufferTy>
		requires std::is_trivially_copyable_v<DataTy> &&
				 std::is_base_of_v<DXBuffer, DXBufferTy>
	class DXBufferWrangler
	{
	public:
		template <typename... DXBufferTyArgs>
		inline DXBufferWrangler(DXBufferTyArgs&&... args) noexcept;

		~DXBufferWrangler() = default;
	public:
		inline void SetData(std::span<DataTy> data);

		inline [[nodiscard]] DXBufferTy& Buffer() noexcept { return m_Buffer; }
		inline [[nodiscard]] const std::vector<DataTy>& Data() { return m_Data; }
	private:
		DXBufferTy m_Buffer;
		std::vector<DataTy> m_Data;
	};

	template <typename DataTy, typename DXBufferTy>
		requires std::is_trivially_copyable_v<DataTy> && std::is_base_of_v<DXBuffer, DXBufferTy>
	template <typename... DXBufferTyArgs>
	inline DXBufferWrangler<DataTy, DXBufferTy>::DXBufferWrangler(DXBufferTyArgs&&... args) noexcept
		: m_Buffer(std::forward<DXBufferTyArgs>(args)...)
	{
	}

	template <typename DataTy, typename DXBufferTy>
		requires std::is_trivially_copyable_v<DataTy> && std::is_base_of_v<DXBuffer, DXBufferTy>
	inline void DXBufferWrangler<DataTy, DXBufferTy>::SetData(std::span<DataTy> data)
	{
		m_Data.assign(data.begin(), data.end());
		m_Buffer.SetData(m_Data);
	}
#pragma endregion
}