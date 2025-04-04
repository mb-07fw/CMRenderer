#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>
#include <string_view>
#include <array>
#include <cstdint>

#include "Internal/Utility/CMStaticArray.hpp"
#include "Internal/Utility/CMLogger.hpp"
#include "Internal/DirectX/DXComponents.hpp"

namespace CMRenderer::CMDirectX
{
	struct DXPos2DInterColorInput
	{
		uint8_t r, g, b, a;
		uint32_t x, y;
	};

	enum class DXImplementedShaderType
	{
		INVALID = -1,
		DEFAULT,
		DEFAULT3D,
		POS2D_INTERCOLOR
	};

	inline constexpr std::array<DXImplementedShaderType, 3> G_IMPLEMENTED_SHADER_TYPES = {
		DXImplementedShaderType::DEFAULT,
		DXImplementedShaderType::DEFAULT3D,
		DXImplementedShaderType::POS2D_INTERCOLOR
	};

	enum class DXShaderType
	{
		INVALID = -1,
		VERTEX, PIXEL
	};

	inline constexpr std::array<DXShaderType, 2> G_SHADER_TYPES = {
		DXShaderType::VERTEX, DXShaderType::PIXEL
	};

	struct DXShaderData
	{
		DXShaderData(
			DXImplementedShaderType implementedType,
			DXShaderType type,
			Microsoft::WRL::ComPtr<ID3DBlob> pShaderBytecode,
			const std::wstring& fileName
		) noexcept;
		~DXShaderData() = default;

		DXImplementedShaderType ImplementedType = DXImplementedShaderType::INVALID;
		DXShaderType Type = DXShaderType::INVALID;
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecode;
		std::wstring Filename;

		static constexpr std::wstring_view ImplementedToWStrView(DXImplementedShaderType implementedType) noexcept;

		static constexpr std::wstring_view S_VERTEX_FLAG = L"VS";
		static constexpr std::wstring_view S_PIXEL_FLAG = L"PS";
	};

	class DXShaderSet
	{
	public:
		DXShaderSet(
			const DXShaderData& vertexData,
			const DXShaderData& pixelData,
			const Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC>& descRef, 
			DXImplementedShaderType implementedType
		) noexcept;

		~DXShaderSet() = default;
	public:
		void CreateShaders(Components::DXDevice& deviceRef, Utility::CMLoggerWide& cmLoggerRef) noexcept;

		inline [[nodiscard]] ID3D11VertexShader* VertexShader() noexcept { return mP_VertexShader.Get(); }
		inline [[nodiscard]] ID3D11PixelShader* PixelShader() noexcept { return mP_PixelShader.Get(); }

		inline [[nodiscard]] const DXShaderData& VertexData() const noexcept { return m_VertexData; }
		inline [[nodiscard]] const DXShaderData& PixelData() const noexcept { return m_PixelData; }

		inline [[nodiscard]] const Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC>& Desc() const noexcept { return m_Desc; }
		inline [[nodiscard]] DXImplementedShaderType ImplementedType() const noexcept { return m_ImplementedType; }

		inline [[nodiscard]] bool IsCreated() const noexcept { return m_Created; }
	private:
		DXShaderData m_VertexData, m_PixelData;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mP_VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mP_PixelShader;
		Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC> m_Desc;
		DXImplementedShaderType m_ImplementedType = DXImplementedShaderType::INVALID;
		bool m_Created = false;
	};

	constexpr std::wstring_view DXShaderData::ImplementedToWStrView(DXImplementedShaderType implementedType) noexcept
	{
		switch (implementedType)
		{
		case DXImplementedShaderType::INVALID:				 return std::wstring_view(L"INVALID");
		case DXImplementedShaderType::DEFAULT:				 return std::wstring_view(L"DEFAULT");
		case DXImplementedShaderType::DEFAULT3D:			 return std::wstring_view(L"DEFAULT3D");
		case DXImplementedShaderType::POS2D_INTERCOLOR:		 return std::wstring_view(L"POS2D_INTERCOLOR");
		default:											 return std::wstring_view(L"NONE");
		}
	}
}