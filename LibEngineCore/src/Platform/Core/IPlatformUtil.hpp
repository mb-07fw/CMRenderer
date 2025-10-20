#pragma once

#include "Platform/Core/IPlatformUUID.hpp"

namespace CMEngine
{
	class IPlatformUtil
	{
	public:
		IPlatformUtil() = default;
		virtual ~IPlatformUtil() = default;

		virtual [[nodiscard]] UUID GenerateUUID() noexcept = 0;
		virtual [[nodiscard]] std::string UUIDToString(const UUID& uiid) noexcept = 0;
	};
}