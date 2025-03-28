#pragma once

#include <string>
#include <string_view>

namespace CMRenderer::Utility
{
	inline const std::wstring BoolToWStr(bool value) noexcept
	{
		if (value)
			return std::wstring(L"TRUE");
		else
			return std::wstring(L"FALSE");
	}

	inline constexpr std::wstring_view BoolToWStrView(bool value) noexcept
	{
		if (value)
			return std::wstring_view(L"TRUE");
		else
			return std::wstring_view(L"FALSE");
	}
}