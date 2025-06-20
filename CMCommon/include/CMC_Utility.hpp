#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <concepts>

#include "CMC_Macros.hpp"

namespace CMCommon::Utility
{
	inline constexpr [[nodiscard]] const wchar_t* BoolToWString(bool value) noexcept
	{
		return (value) ? L"TRUE" : L"FALSE";
	}

	inline constexpr [[nodiscard]] const char* BoolToString(bool value) noexcept
	{
		return (value) ? "TRUE" : "FALSE";
	}

	inline constexpr [[nodiscard]] std::wstring_view BoolToWStringView(bool value) noexcept
	{
		if (value)
			return std::wstring_view(L"TRUE");
		else
			return std::wstring_view(L"FALSE");
	}

	inline [[nodiscard]] std::filesystem::path GetAssetsPath() noexcept
	{
		return std::filesystem::current_path() / "resources/assets/";
	}

	template <std::integral Ty, std::integral ReturnTy = Ty>
	inline constexpr [[nodiscard]] ReturnTy ToPower(Ty value, Ty power) noexcept
	{
		if (power == 0)
			return 1u;

		ReturnTy newValue = 1;

		for (Ty i = 0; i < power; ++i)
			newValue *= value;

		return newValue;
	}

	/* Asserts a condition on debug builds using CM_ASSERT; returns the negated condition. */
	inline [[nodiscard]] bool Assert(bool condition) noexcept
	{
		CM_ASSERT(condition);
		return !condition;
	}

	/* Copies @from into @outTo and converts any alphabetical characters in it to lower case. */
	inline [[nodiscard]] void ToLower(std::string_view from, std::string& outTo) noexcept
	{
		outTo = from;

		for (size_t i = 0; i < outTo.length(); ++i)
		{
			char at = outTo[i];

			outTo[i] = std::tolower(static_cast<unsigned char>(at));
		}
	}

	/* Converts any alphabetical characters in @outStr to lower case. */
	inline [[nodiscard]] void ToLower(std::string& outStr) noexcept
	{
		for (size_t i = 0; i < outStr.length(); ++i)
		{
			char at = outStr[i];

			outStr[i] = std::tolower(static_cast<unsigned char>(at));
		}
	}
}