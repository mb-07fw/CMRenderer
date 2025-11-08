#pragma once

#include <cstdint>

namespace CMEngine
{
	enum class ShaderType : int8_t
	{
		Invalid = -1,
		Vertex,
		Pixel,
		Compute
	};

	enum class AssignedShaderType : int8_t
	{
		Invalid = -1,
		Quad_VS,
		Quad_PS,
		Gltf_Basic_VS,
		Gltf_Basic_PS,
		Custom
	};

	using ShaderUseID = uint8_t;

	struct ShaderID
	{
		inline ShaderID(
			uint32_t index,
			ShaderType type,
			AssignedShaderType assignedType
		)
			: Index(index),
			  Type(type),
			  AssignedType(assignedType)
		{
		}

		ShaderID() = default;
		~ShaderID() = default;

		inline constexpr [[nodiscard]] bool IsValid() const noexcept;
		inline constexpr [[nodiscard]] bool operator==(ShaderID other) const noexcept;
		inline constexpr operator bool() const noexcept { return IsValid(); }

		static constexpr uint32_t S_INVALID_INDEX = ~static_cast<uint32_t>(0);
		uint32_t Index = S_INVALID_INDEX; /* Index into ShaderData list. */
		ShaderType Type = ShaderType::Invalid;
		AssignedShaderType AssignedType = AssignedShaderType::Invalid;
	};

	inline constexpr [[nodiscard]] bool ShaderID::IsValid() const noexcept
	{
		return Index != S_INVALID_INDEX &&
			Type != ShaderType::Invalid &&
			AssignedType != AssignedShaderType::Invalid;
	}

	inline constexpr [[nodiscard]] bool ShaderID::operator==(ShaderID other) const noexcept
	{
		return Index == other.Index &&
			Type == other.Type &&
			AssignedType == other.AssignedType;
	}
}