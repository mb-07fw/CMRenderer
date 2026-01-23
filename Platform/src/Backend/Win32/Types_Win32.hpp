#pragma once

#include <wrl/client.h>

namespace Platform::Backend::Win32
{
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
}