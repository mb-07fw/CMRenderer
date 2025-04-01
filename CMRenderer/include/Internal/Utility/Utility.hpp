#pragma once

#include <string>
#include <string_view>

namespace CMRenderer::Utility
{
	inline constexpr const wchar_t* BoolToWStr(bool value) noexcept
	{
		return (value) ?L"TRUE" : L"FALSE";
	}

	inline constexpr std::wstring_view BoolToWStrView(bool value) noexcept
	{
		if (value)
			return std::wstring_view(L"TRUE");
		else
			return std::wstring_view(L"FALSE");
	}
}