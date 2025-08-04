#pragma once

#include "Core/TypeHint.hpp"
#include "Common/Types.hpp"

#include <string>
#include <string_view>
#include <array>
#include <vector>

namespace CMEngine::Core::Asset
{
	struct MeshConstants
	{
		static constexpr std::array<Common::Float2, 4> S_QUAD_FRONT_VERTICES = { {
			{ -0.5f,  0.5f },
			{  0.5f,  0.5f },
			{  0.5f, -0.5f },
			{ -0.5f, -0.5f }
		} };

		static constexpr std::array<Common::Float3, 4> S_QUAD_FRONT_DEPTH_VERTICES = { {
			{ -0.5f,  0.5f, -0.5f },
			{  0.5f,  0.5f, -0.5f },
			{  0.5f, -0.5f, -0.5f },
			{ -0.5f, -0.5f, -0.5f }
		} };

		static constexpr std::array<Common::Float3, 72> S_CUBE_VERTICES = { {
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

	struct VertexAttribute
	{
		inline VertexAttribute(
			std::string_view name,
			TypeHint typeHint,
			uint32_t byteOffset
		) noexcept;

		VertexAttribute() = default;
		~VertexAttribute() = default;

		std::string Name;
		TypeHint TypeHint = TypeHint::NONE;
		uint32_t ByteOffset = Common::Constants::U32_MAX_VALUE;
	};

	inline VertexAttribute::VertexAttribute(
		std::string_view name,
		Core::TypeHint typeHint,
		uint32_t byteOffset
	) noexcept
		: Name(name),
		  TypeHint(typeHint),
		  ByteOffset(byteOffset)
	{
	}

	struct MeshDescriptor
	{
		std::string MeshName;
		uint32_t VertexCount = Common::Constants::U32_MAX_VALUE;
		uint32_t IndexCount = Common::Constants::U32_MAX_VALUE;
		uint32_t VertexByteStride = Common::Constants::U32_MAX_VALUE;
		uint32_t IndexByteStride = Common::Constants::U32_MAX_VALUE;
		TypeHint VertexByteStrideHint = TypeHint::NONE;
		TypeHint IndexByteStrideHint = TypeHint::NONE;
		uint32_t BinaryBeginBytePos = Common::Constants::U32_MAX_VALUE;
		std::vector<VertexAttribute> Attributes;
	};

	struct MeshData
	{
		MeshDescriptor Descriptor = {};
		std::vector<std::byte> VertexData;
		std::vector<std::byte> IndexData;
	};
}