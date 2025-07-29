#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <span>
#include <vector>
#include <type_traits>

#include "DX/DX11/DX11_Components.hpp"
#include "Common/Macros.hpp"

namespace CMEngine::DX::DX11
{
	inline constexpr D3D11_BIND_FLAG CM_BUFFER_BIND_FLAG_INVALID = static_cast<D3D11_BIND_FLAG>(-1);
	inline constexpr D3D11_USAGE CM_BUFFER_USAGE_INVALID = static_cast<D3D11_USAGE>(-1);
	inline constexpr UINT CM_CONSTANT_BUFFER_REGISTER_SLOT_INVALID = static_cast<UINT>(-1);

	enum class BufferType : uint8_t
	{
		INVALID,
		VERTEX,
		INDEX,
		CONSTANT,
		STRUCTURED
	};

#pragma region Buffers
	struct BufferMetaData
	{
		BufferMetaData() = default;
		~BufferMetaData() = default;

		BufferMetaData(
			UINT bindFlags,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		template <typename DataTy>
			requires std::is_trivially_copyable_v<DataTy>
		inline BufferMetaData(
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

	class Buffer
	{
	public:
		Buffer(BufferType type, BufferMetaData metaData = BufferMetaData()) noexcept;
		Buffer() = default;
		virtual ~Buffer() = default;
	public:
		[[nodiscard]] HRESULT Create(Device& device) noexcept;
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

		inline [[nodiscard]] BufferType Type() const noexcept { return m_Type; }
		inline [[nodiscard]] const BufferMetaData& MetaData() const noexcept { return m_MetaData; }

		inline [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11Buffer> Get() { return mP_Buffer; }
		inline [[nodiscard]] ID3D11Buffer* GetRaw() { return mP_Buffer.Get(); }
		inline [[nodiscard]] ID3D11Buffer** GetAddress() { return mP_Buffer.GetAddressOf(); }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
	protected:
		BufferType m_Type = BufferType::INVALID;
		BufferMetaData m_MetaData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mP_Buffer;
		bool m_Created = false;
		bool m_Released = false;
	};

	class VertexBuffer : public Buffer
	{
	public:
		VertexBuffer(
			UINT vertexStride,
			UINT registerSlot = 0u,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		template <typename DataTy>
			requires std::is_trivially_copyable_v<DataTy>
		inline VertexBuffer(
			UINT vertexStride,
			std::span<DataTy> data,
			UINT registerSlot = 0u,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		VertexBuffer() = default;
		~VertexBuffer() = default;
	public:
		inline static constexpr [[nodiscard]] bool IsValidRegister(UINT slot) noexcept;
		inline static constexpr [[nodiscard]] UINT MaxRegisterSlot() noexcept { return D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1; }

		void Bind(Device& device, UINT offset = 0u) noexcept;
		void SetStride(UINT vertexStride) noexcept;

		inline [[nodiscard]] UINT Stride() const noexcept { return m_Stride; }
	private:
		UINT m_Stride = 0u;
		UINT m_Register = 0u;
	};

	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer(
			DXGI_FORMAT format,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		template <typename DataTy>
			requires std::is_trivially_copyable_v<DataTy>
		inline IndexBuffer(
			DXGI_FORMAT format,
			std::span<DataTy> data,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		IndexBuffer() = default;
		~IndexBuffer() = default;
	public:
		void Bind(Device& device, UINT offset = 0u) noexcept;
		void SetFormat(DXGI_FORMAT format) noexcept;
		
		inline [[nodiscard]] DXGI_FORMAT Format() const noexcept { return m_Format; }
	private:
		DXGI_FORMAT m_Format = DXGI_FORMAT_UNKNOWN;
	};

	class ConstantBuffer : public Buffer
	{
	public:
		ConstantBuffer(
			UINT registerSlot,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		template <typename DataTy>
			requires std::is_trivially_copyable_v<DataTy>
		inline ConstantBuffer(
			UINT registerSlot,
			std::span<DataTy> data,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			UINT cpuAccessFlags = 0u,
			UINT miscFlags = 0u,
			UINT structureByteStride = 0u
		) noexcept;

		ConstantBuffer() = default;
		~ConstantBuffer() = default;
	public:
		inline static constexpr [[nodiscard]] bool IsValidRegister(UINT slot) noexcept;

		void BindVS(Device& device) noexcept;
		void BindPS(Device& device) noexcept;
		void SetRegister(UINT registerSlot) noexcept;

		inline [[nodiscard]] UINT Register() const noexcept { return m_Register; }
	private:
		UINT m_Register = 0u;
	};

	template <typename DataTy>
		requires std::is_trivially_copyable_v<DataTy>
	inline BufferMetaData::BufferMetaData(
		std::span<DataTy> data,
		UINT bindFlags,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Desc(
			static_cast<UINT>(data.size_bytes()),
			bindFlags,
			usage,
			cpuAccessFlags,
			miscFlags,
			structureByteStride)
	{
		SubresourceData.pSysMem = data.data();
	}

	template <typename DataTy>
		requires std::is_trivially_copyable_v<DataTy>
	inline void Buffer::SetData(std::span<DataTy> data) noexcept
	{
		ReleaseIfCreated();

		CM_ASSERT(data.data() != nullptr && data.size_bytes() != 0u);

		m_MetaData.SubresourceData.pSysMem = data.data();
		m_MetaData.Desc.ByteWidth = static_cast<UINT>(data.size_bytes());
	}

	template <typename DataTy>
		requires std::is_trivially_copyable_v<DataTy>
	inline VertexBuffer::VertexBuffer(
		UINT vertexStride,
		std::span<DataTy> data,
		UINT registerSlot,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Buffer(
			BufferType::VERTEX,
			BufferMetaData(
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

	inline constexpr [[nodiscard]] bool VertexBuffer::IsValidRegister(UINT slot) noexcept
	{
		/* The maximum of 16 or 32 input slots (ranges from 0 to D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1)
		 * are available; the maximum number of input slots depends on the feature level.
		 */
		return slot < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
	}

	template <typename DataTy>
		requires std::is_trivially_copyable_v<DataTy>
	inline IndexBuffer::IndexBuffer(
		DXGI_FORMAT format,
		std::span<DataTy> data,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Buffer(
			BufferType::INDEX,
			BufferMetaData( 
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
	inline ConstantBuffer::ConstantBuffer(
		UINT registerSlot,
		std::span<DataTy> data,
		D3D11_USAGE usage,
		UINT cpuAccessFlags,
		UINT miscFlags,
		UINT structureByteStride
	) noexcept
		: Buffer(
			BufferType::CONSTANT,
			BufferMetaData(
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

	inline constexpr [[nodiscard]] bool ConstantBuffer::IsValidRegister(UINT slot) noexcept
	{
		/* Index into the device's zero-based array to begin setting constant buffers to
		 * (ranges from 0 to D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1).
		 */
		return slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	}

	/* Wraps a buffer type, and stores it's data locally to ensure proper lifetime semantics. */
	template <typename DataTy, typename BufferTy>
		requires std::is_trivially_copyable_v<DataTy> &&
				 std::is_base_of_v<Buffer, BufferTy>
	class BufferWrangler
	{
	public:
		template <typename... BufferTyArgs>
		inline BufferWrangler(BufferTyArgs&&... args) noexcept;

		~BufferWrangler() = default;
	public:
		inline void SetData(std::span<DataTy> data);

		inline [[nodiscard]] BufferTy& Buffer() noexcept { return m_Buffer; }
		inline [[nodiscard]] const std::vector<DataTy>& Data() { return m_Data; }
	private:
		BufferTy m_Buffer;
		std::vector<DataTy> m_Data;
	};

	template <typename DataTy, typename BufferTy>
		requires std::is_trivially_copyable_v<DataTy> && std::is_base_of_v<Buffer, BufferTy>
	template <typename... BufferTyArgs>
	inline BufferWrangler<DataTy, BufferTy>::BufferWrangler(BufferTyArgs&&... args) noexcept
		: m_Buffer(std::forward<BufferTyArgs>(args)...)
	{
	}

	template <typename DataTy, typename BufferTy>
		requires std::is_trivially_copyable_v<DataTy> && std::is_base_of_v<Buffer, BufferTy>
	inline void BufferWrangler<DataTy, BufferTy>::SetData(std::span<DataTy> data)
	{
		m_Data.assign(data.begin(), data.end());
		m_Buffer.SetData(m_Data);
	}
#pragma endregion
}