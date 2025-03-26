#pragma once

namespace CMRenderer::Utility
{
	const std::wstring BoolToWStr(bool value) noexcept
	{
		if (value)
			return std::wstring(L"TRUE");
		else
			return std::wstring(L"FALSE");
	}
}