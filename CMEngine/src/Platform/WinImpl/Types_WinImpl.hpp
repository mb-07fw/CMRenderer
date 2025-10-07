#pragma once

#include "Types.hpp"

#include <wrl/client.h>

namespace CMEngine::Platform::WinImpl
{
	template <typename Ty>
	using ComPtr = Microsoft::WRL::ComPtr<Ty>;
}