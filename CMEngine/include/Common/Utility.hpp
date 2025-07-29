#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <concepts>
#include <vector>
#include <array>
#include <span>

#include "Common/Macros.hpp"

namespace CMEngine::Common::Utility
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

			outTo[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(at)));
		}
	}

	/* Converts any alphabetical characters in @outStr to lower case. */
	inline [[nodiscard]] void ToLower(std::string& outStr) noexcept
	{
		for (size_t i = 0; i < outStr.length(); ++i)
		{
			char at = outStr[i];

			outStr[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(at)));
		}
	}

	template <typename Ty>
	inline constexpr [[nodiscard]] std::span<const Ty> EmptySpan() noexcept
	{
		return std::span<const Ty, 0>(nullptr, 0);
	}

	template <typename Ty> 
	inline [[nodiscard]] std::span<const Ty> AsSpan(const std::vector<Ty>& vector) noexcept
	{
		if (vector.size() == 0)
			return EmptySpan<Ty>();
		
		return std::span<const Ty>(vector);
	}

	template <typename Ty, size_t Size>
	inline constexpr [[nodiscard]] std::span<const Ty, Size> AsSpan(const std::array<Ty, Size>& array) noexcept
	{
		return std::span<const Ty, Size>(array);
	}

	template <typename Ty, size_t Size>
	inline constexpr [[nodiscard]] std::span<const Ty, Size> AsSpan(const Ty (&array)[Size]) noexcept
	{
		return std::span<const Ty, Size>(array);
	}

	template <typename Ty>
	inline constexpr [[nodiscard]] std::span<const Ty> AsSpan(const Ty& instance) noexcept
	{
		return std::span<const Ty>(&instance, 1);
	}

	template <typename FromTy, size_t FromExtent>
	inline constexpr [[nodiscard]] std::span<const std::byte> ToBytes(std::span<const FromTy, FromExtent> span) noexcept
	{
		return std::as_bytes(span);
	}

	template <typename Ty, size_t Size>
		requires (!sizeof(Ty) != sizeof(std::byte))
	inline constexpr [[nodiscard]] std::span<const std::byte> ToBytes(const Ty(&array)[Size]) noexcept
	{
		return std::as_bytes(AsSpan(array));
	}
}