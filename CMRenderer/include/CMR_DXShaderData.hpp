#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <cstdint>

#include "CMC_Logger.hpp"
#include "CMR_DXComponents.hpp"

namespace CMRenderer::CMDirectX
{
	enum class DXShaderSetType : int8_t
	{
		INVALID = -1,
		CMRECT,
		CMCUBE
	};

	inline constexpr std::array<DXShaderSetType, 2u> G_IMPLEMENTED_SET_TYPES = {
		DXShaderSetType::CMRECT,
		DXShaderSetType::CMCUBE,
	};

	enum class DXShaderType : int8_t
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
			DXShaderSetType correspondingShaderSet,
			DXShaderType type,
			Microsoft::WRL::ComPtr<ID3DBlob> pShaderBytecode,
			const std::wstring& fileName
		) noexcept;
		DXShaderData() = default;
		~DXShaderData() = default;

		DXShaderSetType CorrespondingShaderSet = DXShaderSetType::INVALID;
		DXShaderType Type = DXShaderType::INVALID;
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecode;
		std::wstring Filename;

		static constexpr std::wstring_view ShaderSetTypeToWStrView(DXShaderSetType setType) noexcept;

		static constexpr std::wstring_view S_VERTEX_FLAG = L"VS";
		static constexpr std::wstring_view S_PIXEL_FLAG = L"PS";
	};

	constexpr std::wstring_view DXShaderData::ShaderSetTypeToWStrView(DXShaderSetType setType) noexcept
	{
		switch (setType)
		{
		case DXShaderSetType::INVALID:		return std::wstring_view(L"INVALID");
		case DXShaderSetType::CMRECT:		return std::wstring_view(L"CMRECT");
		case DXShaderSetType::CMCUBE:		return std::wstring_view(L"CMCUBE");
		default:							return std::wstring_view(L"NONE");
		}
	}
}