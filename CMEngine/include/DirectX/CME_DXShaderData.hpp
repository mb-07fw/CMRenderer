#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <cstdint>

#include "DirectX/CME_DXComponents.hpp"
#include "CMC_Logger.hpp"

namespace CMEngine::DirectXAPI::DX11
{
	enum class DXShaderSetType : uint8_t
	{
		INVALID,
		QUAD,
		QUAD_OUTLINED,
		QUAD_DEPTH,
		CIRCLE
	};

	inline constexpr std::array<DXShaderSetType, 4u> G_IMPLEMENTED_SET_TYPES = {
		DXShaderSetType::QUAD,
		DXShaderSetType::QUAD_OUTLINED,
		DXShaderSetType::QUAD_DEPTH,
		DXShaderSetType::CIRCLE,
	};

	enum class DXShaderType : uint8_t
	{
		INVALID,
		VERTEX,
		PIXEL
	};

	inline constexpr std::array<DXShaderType, 2> G_SHADER_TYPES = {
		DXShaderType::VERTEX,
		DXShaderType::PIXEL
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
		case DXShaderSetType::INVALID:			return std::wstring_view(L"INVALID");
		case DXShaderSetType::QUAD:				return std::wstring_view(L"QUAD");
		case DXShaderSetType::QUAD_OUTLINED:	return std::wstring_view(L"QUAD_OUTLINED");
		case DXShaderSetType::QUAD_DEPTH:		return std::wstring_view(L"QUAD_DEPTH");
		case DXShaderSetType::CIRCLE:			return std::wstring_view(L"CIRCLE");
		default:								return std::wstring_view(L"NONE");
		}
	}
}