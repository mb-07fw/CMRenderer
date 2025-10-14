#pragma once

#include "Platform/Core/IWindow.hpp"
#include "Platform/Core/IGraphics.hpp"

#include "Types.hpp"

#include <string>

namespace CMEngine
{
	class CM_ENGINE_API IPlatform
	{
	public:
		IPlatform() = default;
		virtual ~IPlatform() = default;

		IPlatform(const IPlatform& other) = delete;
		IPlatform& operator=(const IPlatform& other) = delete;

		virtual bool Update() noexcept = 0;

		virtual [[nodiscard]] bool IsRunning() const noexcept = 0;
	};
}