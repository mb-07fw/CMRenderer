#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "CMC_Utility.hpp"

namespace CMEngine
{
	enum class CMTypeHint : uint8_t
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

	inline constexpr std::string_view G_CM_TYPE_HINT_INVALID = "invalid";
	inline constexpr std::string_view G_CM_TYPE_HINT_NONE = "none";
	inline constexpr std::string_view G_CM_TYPE_HINT_UINT8 = "uint8";
	inline constexpr std::string_view G_CM_TYPE_HINT_UINT16 = "uint16";
	inline constexpr std::string_view G_CM_TYPE_HINT_UINT32 = "uint32";
	inline constexpr std::string_view G_CM_TYPE_HINT_FLOAT32 = "float32";
	inline constexpr std::string_view G_CM_TYPE_HINT_FLOAT32x2 = "float32x2";
	inline constexpr std::string_view G_CM_TYPE_HINT_FLOAT32x3 = "float32x3";

	inline constexpr [[nodiscard]] CMTypeHint StringToTypeHint(std::string_view str, bool& parsedTrue) noexcept
	{
		/* Set flag to true early to prevent setting it in every case later... */
		parsedTrue = true;

		/* Return early if @str.length() is less than smallest type hint length. */
		if (str.length() < G_CM_TYPE_HINT_NONE.length())
		{
			parsedTrue = false;
			return CMTypeHint::INVALID;
		}

		/* Turn @str to lowercase to make string comparison less painful. */
		std::string copyStr;
		CMCommon::Utility::ToLower(str, copyStr);

		if (copyStr == G_CM_TYPE_HINT_INVALID)         return CMTypeHint::INVALID;
		else if (copyStr == G_CM_TYPE_HINT_NONE)       return CMTypeHint::NONE;
		else if (copyStr == G_CM_TYPE_HINT_UINT8)      return CMTypeHint::UINT8;
		else if (copyStr == G_CM_TYPE_HINT_UINT16)     return CMTypeHint::UINT16;
		else if (copyStr == G_CM_TYPE_HINT_UINT32)     return CMTypeHint::UINT32;
		else if (copyStr == G_CM_TYPE_HINT_FLOAT32)    return CMTypeHint::FLOAT32;
		else if (copyStr == G_CM_TYPE_HINT_FLOAT32x2)  return CMTypeHint::FLOAT32x2;
		else if (copyStr == G_CM_TYPE_HINT_FLOAT32x3)  return CMTypeHint::FLOAT32x3;

		parsedTrue = false;
		return CMTypeHint::INVALID;
	}

	inline constexpr [[nodiscard]] std::string_view TypeHintToString(CMTypeHint typeHint) noexcept
	{
		switch (typeHint)
		{
		case CMTypeHint::INVALID:
			return G_CM_TYPE_HINT_INVALID;
		case CMTypeHint::NONE:
			return G_CM_TYPE_HINT_NONE;
		case CMTypeHint::UINT8:
			return G_CM_TYPE_HINT_UINT8;
		case CMTypeHint::UINT16:
			return G_CM_TYPE_HINT_UINT16;
		case CMTypeHint::UINT32:
			return G_CM_TYPE_HINT_UINT32;
		case CMTypeHint::FLOAT32:
			return G_CM_TYPE_HINT_FLOAT32;
		case CMTypeHint::FLOAT32x2:
			return G_CM_TYPE_HINT_FLOAT32x2;
		case CMTypeHint::FLOAT32x3:
			return G_CM_TYPE_HINT_FLOAT32x3;
		default:
			return "unknown";
		}
	}

	inline constexpr [[nodiscard]] uint32_t ToByteSize(CMTypeHint typeHint) noexcept
	{
		switch (typeHint)
		{
		case CMTypeHint::UINT8:      return sizeof(uint8_t);
		case CMTypeHint::UINT16:     return sizeof(uint16_t);
		case CMTypeHint::UINT32:     return sizeof(uint32_t);
		case CMTypeHint::FLOAT32:    return sizeof(float);
		case CMTypeHint::FLOAT32x2:  return sizeof(float) * 2;
		case CMTypeHint::FLOAT32x3:  return sizeof(float) * 3;
		case CMTypeHint::INVALID: [[fallthrough]];
		case CMTypeHint::NONE: [[fallthrough]];
		default:                     return 0;
		}
	}
}