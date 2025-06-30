#pragma once

#include "Core/CME_TypeHint.hpp"
#include "CMC_Types.hpp"

#include <string>
#include <string_view>
#include <array>
#include <vector>

namespace CMEngine::Asset
{
	struct CMMeshConstants
	{
		static constexpr std::array<CMCommon::CMFloat2, 4> S_QUAD_FRONT_VERTICES = { {
			{ -0.5f,  0.5f },
			{  0.5f,  0.5f },
			{  0.5f, -0.5f },
			{ -0.5f, -0.5f }
		} };

		static constexpr std::array<CMCommon::CMFloat3, 4> S_QUAD_FRONT_DEPTH_VERTICES = { {
			{ -0.5f,  0.5f, -0.5f },
			{  0.5f,  0.5f, -0.5f },
			{  0.5f, -0.5f, -0.5f },
			{ -0.5f, -0.5f, -0.5f }
		} };

		static constexpr std::array<CMCommon::CMFloat3, 72> S_CUBE_VERTICES = { {
				// Front face
				{ -1.0f,  1.0f, -1.0f },
				{  1.0f,  1.0f, -1.0f },
				{  1.0f, -1.0f, -1.0f },
				{ -1.0f, -1.0f, -1.0f },

				// Back face
				{ -1.0f,  1.0f,  1.0f },
				{  1.0f,  1.0f,  1.0f },
				{  1.0f, -1.0f,  1.0f },
				{ -1.0f, -1.0f,  1.0f },

				// Left face
				{ -1.0f,  1.0f, -1.0f },
				{ -1.0f,  1.0f,  1.0f },
				{ -1.0f, -1.0f,  1.0f },
				{ -1.0f, -1.0f, -1.0f},

				// Right face
				{ 1.0f,  1.0f, -1.0f },
				{ 1.0f,  1.0f,  1.0f },
				{ 1.0f, -1.0f,  1.0f },
				{ 1.0f, -1.0f, -1.0f },

				// Top face
				{ -1.0f,  1.0f,  1.0f },
				{  1.0f,  1.0f,  1.0f },
				{  1.0f,  1.0f, -1.0f },
				{ -1.0f,  1.0f, -1.0f },

				// Bottom face
				{ -1.0f, -1.0f,  1.0f },
				{  1.0f, -1.0f,  1.0f },
				{  1.0f, -1.0f, -1.0f },
				{ -1.0f, -1.0f, -1.0f }
			} };

		static constexpr std::array<uint16_t, 6> S_QUAD_FRONT_INDICES = {
			0, 1, 2,
			0, 2, 3,
		};

		static constexpr std::array<uint16_t, 36> S_CUBE_INDICES = { {
			0, 1, 2,
			0, 2, 3,

			6, 5, 4,
			7, 6, 4,

			10, 9, 8,
			11, 10, 8,

			12, 13, 14,
			12, 14, 15,

			16, 17, 18,
			16, 18, 19,

			22, 21, 20,
			23, 22, 20
		} };
	};

	struct CMVertexAttribute
	{
		inline CMVertexAttribute(
			std::string_view name,
			CMTypeHint typeHint,
			uint32_t byteOffset
		) noexcept;

		CMVertexAttribute() = default;
		~CMVertexAttribute() = default;

		std::string Name;
		CMTypeHint TypeHint = CMTypeHint::NONE;
		uint32_t ByteOffset = CMCommon::Constants::U32_MAX_VALUE;
	};

	inline CMVertexAttribute::CMVertexAttribute(
		std::string_view name,
		CMTypeHint typeHint,
		uint32_t byteOffset
	) noexcept
		: Name(name),
		  TypeHint(typeHint),
		  ByteOffset(byteOffset)
	{
	}

	struct CMMeshDescriptor
	{
		std::string MeshName;
		uint32_t VertexCount = CMCommon::Constants::U32_MAX_VALUE;
		uint32_t IndexCount = CMCommon::Constants::U32_MAX_VALUE;
		uint32_t VertexByteStride = CMCommon::Constants::U32_MAX_VALUE;
		uint32_t IndexByteStride = CMCommon::Constants::U32_MAX_VALUE;
		CMTypeHint VertexByteStrideHint = CMTypeHint::NONE;
		CMTypeHint IndexByteStrideHint = CMTypeHint::NONE;
		uint32_t BinaryBeginBytePos = CMCommon::Constants::U32_MAX_VALUE;
		std::vector<CMVertexAttribute> Attributes;
	};

	struct CMMeshData
	{
		CMMeshDescriptor Descriptor = {};
		std::vector<std::byte> VertexData;
		std::vector<std::byte> IndexData;
	};
}