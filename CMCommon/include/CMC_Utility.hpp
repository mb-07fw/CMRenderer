#pragma once

#include <string>
#include <string_view>

namespace CMRenderer::Utility
{
	inline constexpr [[nodiscard]] const wchar_t* BoolToWStr(bool value) noexcept
	{
		return (value) ? L"TRUE" : L"FALSE";
	}

	inline constexpr [[nodiscard]] const char* BoolToStr(bool value) noexcept
	{
		return (value) ? "TRUE" : "FALSE";
	}

	inline constexpr [[nodiscard]] std::wstring_view BoolToWStrView(bool value) noexcept
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
}