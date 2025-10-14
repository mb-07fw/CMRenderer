#include "PCH.hpp"
#include "Platform/WinImpl/PlatformUtil_WinImpl.hpp"
#include "Macros.hpp"

namespace CMEngine::Platform::WinImpl
{
	[[nodiscard]] UUID PlatformUtil::GenerateUUID() noexcept
	{
		::GUID guid = {};

		HRESULT hr = CoCreateGuid(&guid);
		CM_ENGINE_ASSERT(!FAILED(hr));

		UUID uuid = {};
		ToEngineUUID(guid, uuid);

		return uuid;
	}

	[[nodiscard]] std::string PlatformUtil::UUIDToString(const UUID& uuid) noexcept
	{
		::GUID winGUID;
		ToWinUUID(uuid, winGUID);

		RPC_CSTR pString = nullptr;
		CM_ENGINE_ASSERT(UuidToStringA(&winGUID, &pString) == RPC_S_OK);

		std::string string(reinterpret_cast<const char*>(pString));
		RpcStringFreeA(&pString);

		return string;
	}

	void PlatformUtil::ToEngineUUID(const ::GUID& winGUID, UUID& outUUID) noexcept
	{
		static_assert(sizeof(::GUID) == sizeof(UUID), "Sizeof WinAPI GUID doesn't match CMEngine::UUID.");

		std::memcpy(&outUUID, &winGUID, sizeof(winGUID));
	}

	void PlatformUtil::ToWinUUID(const UUID& uuid, ::GUID& outWinGUID) noexcept
	{
		static_assert(sizeof(::GUID) == sizeof(UUID), "Sizeof WinAPI GUID doesn't match CMEngine::UUID.");

		std::memcpy(&outWinGUID, &uuid, sizeof(uuid));
	}
}