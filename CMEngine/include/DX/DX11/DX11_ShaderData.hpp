#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <cstdint>

#include <d3d11.h>
#include <wrl/client.h>

#include "DX/DX11/DX11_Components.hpp"
#include "Common/Logger.hpp"

namespace CMEngine::DX::DX11
{
	enum class ShaderSetType : uint8_t
	{
		INVALID,
		QUAD,
		QUAD_OUTLINED,
		QUAD_DEPTH,
		CIRCLE
	};

	inline constexpr std::array<ShaderSetType, 4u> G_IMPLEMENTED_SET_TYPES = {
		ShaderSetType::QUAD,
		ShaderSetType::QUAD_OUTLINED,
		ShaderSetType::QUAD_DEPTH,
		ShaderSetType::CIRCLE,
	};

	enum class ShaderType : uint8_t
	{
		INVALID,
		VERTEX,
		PIXEL
	};

	inline constexpr std::array<ShaderType, 2> G_SHADER_TYPES = {
		ShaderType::VERTEX,
		ShaderType::PIXEL
	};

	struct ShaderData
	{
		ShaderData(
			ShaderSetType correspondingShaderSet,
			ShaderType type,
			Microsoft::WRL::ComPtr<ID3DBlob> pShaderBytecode,
			const std::wstring& fileName
		) noexcept;
		ShaderData() = default;
		~ShaderData() = default;

		ShaderSetType CorrespondingShaderSet = ShaderSetType::INVALID;
		ShaderType Type = ShaderType::INVALID;
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecode;
		std::wstring Filename;

		static constexpr std::wstring_view ShaderSetTypeToWStrView(ShaderSetType setType) noexcept;

		static constexpr std::wstring_view S_VERTEX_FLAG = L"VS";
		static constexpr std::wstring_view S_PIXEL_FLAG = L"PS";
	};

	constexpr std::wstring_view ShaderData::ShaderSetTypeToWStrView(ShaderSetType setType) noexcept
	{
		switch (setType)
		{
		case ShaderSetType::INVALID:		return std::wstring_view(L"INVALID");
		case ShaderSetType::QUAD:			return std::wstring_view(L"QUAD");
		case ShaderSetType::QUAD_OUTLINED:	return std::wstring_view(L"QUAD_OUTLINED");
		case ShaderSetType::QUAD_DEPTH:		return std::wstring_view(L"QUAD_DEPTH");
		case ShaderSetType::CIRCLE:			return std::wstring_view(L"CIRCLE");
		default:							return std::wstring_view(L"NONE");
		}
	}
}