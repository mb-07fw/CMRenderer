#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "Common/Utility.hpp"

namespace CMEngine::Core
{
	enum class TypeHint : uint8_t
	{
		INVALID,
		NONE,
		UINT8,
		UINT16,
		UINT32,
		FLOAT32,
		FLOAT32x2,
		FLOAT32x3
	};

	/* Note for later :
	 *
	 *   If you choose to add more type hints, add them in every other function here as well.
	 *
	 *
	 *
	 *
	 *   Or else...
	 */

	inline constexpr std::string_view G_TYPE_HINT_INVALID = "invalid";
	inline constexpr std::string_view G_TYPE_HINT_NONE = "none";
	inline constexpr std::string_view G_TYPE_HINT_UINT8 = "uint8";
	inline constexpr std::string_view G_TYPE_HINT_UINT16 = "uint16";
	inline constexpr std::string_view G_TYPE_HINT_UINT32 = "uint32";
	inline constexpr std::string_view G_TYPE_HINT_FLOAT32 = "float32";
	inline constexpr std::string_view G_TYPE_HINT_FLOAT32x2 = "float32x2";
	inline constexpr std::string_view G_TYPE_HINT_FLOAT32x3 = "float32x3";

	inline constexpr [[nodiscard]] TypeHint StringToTypeHint(std::string_view str, bool& parsedTrue) noexcept
	{
		/* Set flag to true early to prevent setting it in every case later... */
		parsedTrue = true;

		/* Return early if @str.length() is less than smallest type hint length. */
		if (str.length() < G_TYPE_HINT_NONE.length())
		{
			parsedTrue = false;
			return TypeHint::INVALID;
		}

		/* Turn @str to lowercase to make string comparison less painful. */
		std::string copyStr;
		Common::Utility::ToLower(str, copyStr);

		if (copyStr == G_TYPE_HINT_INVALID)         return TypeHint::INVALID;
		else if (copyStr == G_TYPE_HINT_NONE)       return TypeHint::NONE;
		else if (copyStr == G_TYPE_HINT_UINT8)      return TypeHint::UINT8;
		else if (copyStr == G_TYPE_HINT_UINT16)     return TypeHint::UINT16;
		else if (copyStr == G_TYPE_HINT_UINT32)     return TypeHint::UINT32;
		else if (copyStr == G_TYPE_HINT_FLOAT32)    return TypeHint::FLOAT32;
		else if (copyStr == G_TYPE_HINT_FLOAT32x2)  return TypeHint::FLOAT32x2;
		else if (copyStr == G_TYPE_HINT_FLOAT32x3)  return TypeHint::FLOAT32x3;

		parsedTrue = false;
		return TypeHint::INVALID;
	}

	inline constexpr [[nodiscard]] std::string_view TypeHintToString(TypeHint typeHint) noexcept
	{
		switch (typeHint)
		{
		case TypeHint::INVALID:   return G_TYPE_HINT_INVALID;
		case TypeHint::NONE:      return G_TYPE_HINT_NONE;
		case TypeHint::UINT8:     return G_TYPE_HINT_UINT8;
		case TypeHint::UINT16:    return G_TYPE_HINT_UINT16;
		case TypeHint::UINT32:    return G_TYPE_HINT_UINT32;
		case TypeHint::FLOAT32:   return G_TYPE_HINT_FLOAT32;
		case TypeHint::FLOAT32x2: return G_TYPE_HINT_FLOAT32x2;
		case TypeHint::FLOAT32x3: return G_TYPE_HINT_FLOAT32x3;
		default:				  return "unknown";
		}
	}

	inline constexpr [[nodiscard]] uint32_t ToByteSize(TypeHint typeHint) noexcept
	{
		switch (typeHint)
		{
		case TypeHint::UINT8:      return sizeof(uint8_t);
		case TypeHint::UINT16:     return sizeof(uint16_t);
		case TypeHint::UINT32:     return sizeof(uint32_t);
		case TypeHint::FLOAT32:    return sizeof(float);
		case TypeHint::FLOAT32x2:  return sizeof(float) * 2;
		case TypeHint::FLOAT32x3:  return sizeof(float) * 3;
		case TypeHint::INVALID:	   [[fallthrough]];
		case TypeHint::NONE:	   [[fallthrough]];
		default:                   return 0;
		}
	}
}