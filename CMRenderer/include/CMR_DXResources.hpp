#pragma once

#include <span>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>

#include "CMR_DXComponents.hpp"
#include "CMC_Macros.hpp"

namespace CMRenderer::CMDirectX
{
	enum class DXBufferType : uint8_t
	{
		INVALID = 0,
		VERTEX,
		INDEX,
		CONSTANT
	};

#pragma region Buffers
	template <typename DataTy>
	class IDXBuffer
	{
	public:
		inline IDXBuffer(std::span<DataTy> data, DXBufferType type, D3D11_BIND_FLAG bindFlag) noexcept;
		IDXBuffer() = default;
		virtual ~IDXBuffer() = default;
	public:
		inline [[nodiscard]] HRESULT Create(Components::DXDevice& deviceRef) noexcept;
		inline void Release() noexcept;
		inline void ReleaseIfCreated() noexcept;

		inline virtual void Bind(Components::DXDevice& deviceRef) noexcept = 0;

		inline [[nodiscard]] const CD3D11_BUFFER_DESC& Desc() const noexcept { return m_Desc; }
		inline [[nodiscard]] const D3D11_SUBRESOURCE_DATA& SubResourceData() const noexcept { return m_SubResourceData; }

		inline [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer() noexcept { return mP_Buffer; }
		inline [[nodiscard]] ID3D11Buffer* BufferRaw() noexcept { return mP_Buffer.Get(); }
		inline [[nodiscard]] ID3D11Buffer** BufferAddress() noexcept { return mP_Buffer.GetAddressOf(); }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
		inline [[nodiscard]] bool IsReleased() const noexcept { return m_Released; }
		inline [[nodiscard]] bool WasSet() const noexcept { return m_WasSet; }
	protected:
		inline void SetBufferData(std::span<DataTy> data, D3D11_BIND_FLAG bindFlag) noexcept;
	protected:
		CD3D11_BUFFER_DESC m_Desc;
		D3D11_SUBRESOURCE_DATA m_SubResourceData = {};
		Microsoft::WRL::ComPtr<ID3D11Buffer> mP_Buffer;
		std::vector<DataTy> m_Data;
		bool m_Created = false;
		bool m_Released = false;
		bool m_WasSet = false;
	};

	template <typename DataTy>
	class DXVertexBuffer : public IDXBuffer<DataTy>
	{
	public:
		using IDXBuffer<DataTy>::m_Data;
		using IDXBuffer<DataTy>::mP_Buffer;
		using IDXBuffer<DataTy>::IsCreated;
		using IDXBuffer<DataTy>::IsReleased;
		using IDXBuffer<DataTy>::Desc;
		using IDXBuffer<DataTy>::SubResourceData;
		using IDXBuffer<DataTy>::SetBufferData;
		using IDXBuffer<DataTy>::Create;
		using IDXBuffer<DataTy>::Release;
		using IDXBuffer<DataTy>::ReleaseIfCreated;
	public:
		inline DXVertexBuffer(std::span<DataTy> data, UINT stride, UINT offset) noexcept;
		DXVertexBuffer() = default;
		~DXVertexBuffer() = default;
	public:
		inline virtual void Bind(Components::DXDevice& deviceRef) noexcept override;
		inline void Set(std::span<DataTy> data, UINT stride, UINT offset) noexcept;
	private:
		UINT m_Stride = 0;
		UINT m_Offset = 0;
	};

	template <typename DataTy>
	class DXIndexBuffer : public IDXBuffer<DataTy>
	{
	public:
		using IDXBuffer<DataTy>::m_Data;
		using IDXBuffer<DataTy>::mP_Buffer;
		using IDXBuffer<DataTy>::IsCreated;
		using IDXBuffer<DataTy>::IsReleased;
		using IDXBuffer<DataTy>::Desc;
		using IDXBuffer<DataTy>::SubResourceData;
		using IDXBuffer<DataTy>::SetBufferData;
		using IDXBuffer<DataTy>::Create;
		using IDXBuffer<DataTy>::Release;
		using IDXBuffer<DataTy>::ReleaseIfCreated;
	public:
		inline DXIndexBuffer(std::span<DataTy> data, DXGI_FORMAT format, UINT offset) noexcept;
		DXIndexBuffer() = default;
		~DXIndexBuffer() = default;
	public:
		inline virtual void Bind(Components::DXDevice& deviceRef) noexcept override;
		inline void Set(std::span<DataTy> data, DXGI_FORMAT format, UINT offset) noexcept;
	private:
		DXGI_FORMAT m_Format = -1;
		UINT m_Offset = 0;
	};

	template <typename DataTy>
	class DXConstantBuffer : public IDXBuffer<DataTy>
	{
	public:
		using IDXBuffer<DataTy>::m_Data;
		using IDXBuffer<DataTy>::mP_Buffer;
		using IDXBuffer<DataTy>::IsCreated;
		using IDXBuffer<DataTy>::IsReleased;
		using IDXBuffer<DataTy>::Desc;
		using IDXBuffer<DataTy>::SubResourceData;
		using IDXBuffer<DataTy>::SetBufferData;
		using IDXBuffer<DataTy>::Create;
		using IDXBuffer<DataTy>::Release;
		using IDXBuffer<DataTy>::ReleaseIfCreated;
	public:
		inline DXConstantBuffer(std::span<DataTy> data, UINT registerSlot) noexcept;
		DXConstantBuffer() = default;
		~DXConstantBuffer() = default;
	public:
		inline virtual void Bind(Components::DXDevice& deviceRef) noexcept override;
		inline void Set(std::span<DataTy> data, UINT registerSlot) noexcept;
	private:
		void SetRegister(UINT registerSlot) noexcept;
	private:
		UINT m_RegisterSlot = 0;
	};
#pragma endregion

#pragma region Implementations
	template <typename DataTy>
	inline IDXBuffer<DataTy>::IDXBuffer(std::span<DataTy> data, DXBufferType type, D3D11_BIND_FLAG bindFlag) noexcept
		: m_Desc(static_cast<UINT>(data.size_bytes()), bindFlag)
	{
		m_Data.assign(data.begin(), data.end());
		m_SubResourceData.pSysMem = data.data();
	}

	template <typename DataTy>
	inline [[nodiscard]] HRESULT IDXBuffer<DataTy>::Create(Components::DXDevice& deviceRef) noexcept
	{
		CM_ASSERT(deviceRef.IsCreated());
		CM_ASSERT(!IsCreated());

		HRESULT hResult = deviceRef->CreateBuffer(&m_Desc, &m_SubResourceData, &mP_Buffer);

		m_Created = true;
		m_Released = false;

		return hResult;
	}

	template <typename DataTy>
	inline void IDXBuffer<DataTy>::Release() noexcept
	{
		m_Desc = {};
		m_SubResourceData = {};

		m_Data.clear();
		mP_Buffer.Reset();

		m_Created = false;
		m_Released = true;
	}

	template <typename DataTy>
	inline void IDXBuffer<DataTy>::ReleaseIfCreated() noexcept
	{
		if (m_Created)
			Release();
	}

	template <typename DataTy>
	inline void IDXBuffer<DataTy>::SetBufferData(std::span<DataTy> data, D3D11_BIND_FLAG bindFlag) noexcept
	{
		CM_ASSERT(data.data() != nullptr);

		ReleaseIfCreated();

		m_Desc = CD3D11_BUFFER_DESC(static_cast<UINT>(data.size_bytes()), bindFlag);

		m_SubResourceData = {};
		m_SubResourceData.pSysMem = data.data();

		m_Data.assign(data.begin(), data.end());

		m_WasSet = true;
	}

	template <typename DataTy>
	inline DXVertexBuffer<DataTy>::DXVertexBuffer(std::span<DataTy> data, UINT stride, UINT offset) noexcept
		: IDXBuffer<DataTy>(data, DXBufferType::VERTEX, D3D11_BIND_VERTEX_BUFFER),
		m_Stride(stride),
		m_Offset(offset)
	{
	}

	template <typename DataTy>
	inline void DXVertexBuffer<DataTy>::Bind(Components::DXDevice& deviceRef) noexcept
	{
		CM_ASSERT(deviceRef.IsCreated());
		CM_ASSERT(IsCreated());

		deviceRef.ContextRaw()->IASetVertexBuffers(0, 1, mP_Buffer.GetAddressOf(), &m_Stride, &m_Offset);
	}

	template <typename DataTy>
	inline void DXVertexBuffer<DataTy>::Set(std::span<DataTy> data, UINT stride, UINT offset) noexcept
	{
		SetBufferData(data, D3D11_BIND_VERTEX_BUFFER);

		m_Stride = stride;
		m_Offset = offset;
	}

	template <typename DataTy>
	inline DXIndexBuffer<DataTy>::DXIndexBuffer(std::span<DataTy> data, DXGI_FORMAT format, UINT offset) noexcept
		: IDXBuffer<DataTy>(data, DXBufferType::INDEX, D3D11_BIND_INDEX_BUFFER),
		  m_Format(format),
		  m_Offset(offset)
	{
	}

	template <typename DataTy>
	inline void DXIndexBuffer<DataTy>::Bind(Components::DXDevice& deviceRef) noexcept
	{
		CM_ASSERT(deviceRef.IsCreated());
		CM_ASSERT(IsCreated());

		deviceRef.ContextRaw()->IASetIndexBuffer(mP_Buffer.Get(), m_Format, m_Offset);
	}

	template <typename DataTy>
	inline void DXIndexBuffer<DataTy>::Set(std::span<DataTy> data, DXGI_FORMAT format, UINT offset) noexcept
	{
		SetBufferData(data, D3D11_BIND_INDEX_BUFFER);

		m_Format = format;
		m_Offset = offset;
	}

	template <typename DataTy>
	inline DXConstantBuffer<DataTy>::DXConstantBuffer(std::span<DataTy> data, UINT registerSlot) noexcept
		: IDXBuffer<DataTy>(data, DXBufferType::CONSTANT, D3D11_BIND_CONSTANT_BUFFER)
	{
		SetRegister(registerSlot);
	}

	template <typename DataTy>
	inline void DXConstantBuffer<DataTy>::Bind(Components::DXDevice& deviceRef) noexcept
	{
		CM_ASSERT(deviceRef.IsCreated());
		CM_ASSERT(IsCreated());

		deviceRef.ContextRaw()->VSSetConstantBuffers(m_RegisterSlot, 1, mP_Buffer.GetAddressOf());
	}

	template <typename DataTy>
	inline void DXConstantBuffer<DataTy>::Set(std::span<DataTy> data, UINT registerSlot) noexcept
	{
		SetBufferData(data, D3D11_BIND_INDEX_BUFFER);
		SetRegister(registerSlot);
	}

	template <typename DataTy>
	inline void DXConstantBuffer<DataTy>::SetRegister(UINT registerSlot) noexcept
	{
		// Valid register slot range from [0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1]
		CM_ASSERT(registerSlot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);

		m_RegisterSlot = registerSlot;
	}

#pragma endregion
}