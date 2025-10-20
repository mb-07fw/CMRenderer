#pragma once

#include "Platform/Core/IPlatformUtil.hpp"

#include <guiddef.h>

namespace CMEngine::Platform::WinImpl
{
	class PlatformUtil : public IPlatformUtil
	{
	public:
		PlatformUtil() = default;
		~PlatformUtil() = default;

		virtual [[nodiscard]] UUID GenerateUUID() noexcept override;
		virtual [[nodiscard]] std::string UUIDToString(const UUID& guid) noexcept override;
	private:
		void ToEngineUUID(const ::GUID& winGUID, UUID& outUUID) noexcept;
		void ToWinUUID(const UUID& uuid, ::GUID& outWinGUID) noexcept;
	};
}