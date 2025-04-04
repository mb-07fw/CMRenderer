#pragma once

#include <wrl/client.h>
#include <d3dcommon.h>

#include <string>
#include <string_view>
#include <array>
#include <cstdint>

#include "Internal/Utility/CMStaticArray.hpp"

namespace CMRenderer::CMDirectX
{
	struct CMPos2DInterColorInput
	{
		uint8_t r, g, b, a;
		uint32_t x, y;
	};

	enum class CMImplementedShaderType
	{
		INVALID = -1,
		DEFAULT,
		DEFAULT3D,
		POS2D_INTERCOLOR
	};

	inline constexpr std::array<CMImplementedShaderType, 3> G_IMPLEMENTED_SHADER_TYPES = {
		CMImplementedShaderType::DEFAULT,
		CMImplementedShaderType::DEFAULT3D,
		CMImplementedShaderType::POS2D_INTERCOLOR
	};

	enum class CMShaderType
	{
		INVALID = -1,
		VERTEX, PIXEL
	};

	inline constexpr std::array<CMShaderType, 2> G_SHADER_TYPES = {
		CMShaderType::VERTEX, CMShaderType::PIXEL
	};

	struct CMShaderData
	{
		CMShaderData(
			CMImplementedShaderType implementedType,
			CMShaderType type,
			Microsoft::WRL::ComPtr<ID3DBlob> pShaderBytecode,
			const std::wstring& fileName
		) noexcept;
		~CMShaderData() = default;

		CMImplementedShaderType ImplementedType = CMImplementedShaderType::INVALID;
		CMShaderType Type = CMShaderType::INVALID;
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecode;
		std::wstring Filename;

		static constexpr std::wstring_view ImplementedToWStrView(CMImplementedShaderType implementedType) noexcept;

		static constexpr std::wstring_view S_VERTEX_FLAG = L"VS";
		static constexpr std::wstring_view S_PIXEL_FLAG = L"PS";
	};

	class CMShaderSet
	{
	public:
		CMShaderSet(
			const CMShaderData& vertexData,
			const CMShaderData& pixelData,
			const Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC>& descRef, 
			CMImplementedShaderType implementedType
		) noexcept;

		~CMShaderSet() = default;
	public:
		inline [[nodiscard]] const CMShaderData& VertexData() const noexcept { return m_VertexData; }
		inline [[nodiscard]] const CMShaderData& PixelData() const noexcept { return m_PixelData; }
		inline [[nodiscard]] const Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC>& Desc() const noexcept { return m_Desc; }
		inline [[nodiscard]] CMImplementedShaderType ImplementedType() const noexcept { return m_ImplementedType; }
	private:
		CMShaderData m_VertexData, m_PixelData;
		Utility::CMStaticArray<D3D11_INPUT_ELEMENT_DESC> m_Desc;
		CMImplementedShaderType m_ImplementedType;
		
	};

	constexpr std::wstring_view CMShaderData::ImplementedToWStrView(CMImplementedShaderType implementedType) noexcept
	{
		switch (implementedType)
		{
		case CMImplementedShaderType::INVALID:				 return std::wstring_view(L"INVALID");
		case CMImplementedShaderType::DEFAULT:				 return std::wstring_view(L"DEFAULT");
		case CMImplementedShaderType::DEFAULT3D:			 return std::wstring_view(L"DEFAULT3D");
		case CMImplementedShaderType::POS2D_INTERCOLOR:		 return std::wstring_view(L"POS2D_INTERCOLOR");
		default:											 return std::wstring_view(L"NONE");
		}
	}
}