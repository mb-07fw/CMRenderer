#include "Asset/AssetID.hpp"
#include "Common/Assert.hpp"
#include "Common/Cast.hpp"

namespace Engine::Asset
{
	AssetID::AssetID(AssetType type, uint32_t globalID) noexcept
	{
		SetType(type);
		SetGlobalID(globalID);
	}

	[[nodiscard]] bool AssetID::IsRegistered() const noexcept
	{
		/* Mask out the value in the IsRegistered field (bit 63).
		 * Shift it down from bit 63 to bit 0 to produce a boolean value. */
		return Cast<bool>((m_Handle & S_IsRegistered_Mask) >> S_IsRegistered_Shift);
	}

	[[nodiscard]] AssetType AssetID::Type() const noexcept
	{
		/* Mask out the value in the AssetType field (bits 56 - 62).
		 * Shift it down to get the original enum value.
		 */
		return Cast<AssetType>((m_Handle & S_AssetType_Mask) >> S_AssetType_Shift);
	}

	[[nodiscard]] uint32_t AssetID::GlobalID() const noexcept
	{
		/* Mask out the value in the GlobalID field (bits 0 - 31).
		 * No shift needed, as the GlobalID field is already aligned at bit 0. */
		return m_Handle & S_GlobalID_Mask;
	}

	[[nodiscard]] AssetID::View AssetID::ToView() const noexcept
	{
		return View(*this);
	}

	[[nodiscard]] bool AssetID::operator==(AssetID other) const noexcept
	{
		return m_Handle == other.m_Handle;
	}

	[[nodiscard]] bool AssetID::operator<(AssetID other) const noexcept
	{
		return this->m_Handle < other.m_Handle;
	}

	void AssetID::SetRegistered(bool isRegistered) noexcept
	{
		/* Clear the IsRegistered bit (bit 63) :
		 *    (Handle &~ G__ASSET_HANDLE_REGISTERED_MASK)
		 *
		 * Cast isRegistered to a uint32_t for correct bitwise shifting.
		 * Shift the boolean value up to its correct position (0th bit to 63rd bit)
		 * No need to mask the value since it will always be 0 or 1 and will never overflow into other bits.
		 * Set the new IsRegistered bit by bitwise OR'ing. */
		m_Handle = (m_Handle & ~S_IsRegistered_Mask) |
			(Cast<uint32_t>(isRegistered) << S_IsRegistered_Shift);
	}

	void AssetID::SetType(AssetType type) noexcept
	{
		ASSERT(IsValidAssetType(type), "(AssetID) Attempted to set AssetType with an unsupported value.");

		/* Clear the AssetType field :
		 *    (Handle &~ S_ASSET_TYPE_MASK)
		 *
		 * Cast type to a uint32_t for correct bitwise shifting.
		 * Shift the value up to its position in the field (bits 56 - 62).
		 * Mask the shifted value so that only the 7 bits that correspond to the type field remain � all higher-order bits are zeroed.
		 * Set the new AssetType bits by bitwise OR'ing. */
		m_Handle = (m_Handle & ~S_AssetType_Mask) |
			((Cast<uint32_t>(type) << S_AssetType_Shift) & S_AssetType_Mask);
	}

	void AssetID::SetGlobalID(uint32_t globalID) noexcept
	{
		ASSERT(globalID < S_GlobalID_Maximum, "(AssetID) Attempted to set a GlobalID that overflows the bitfield.");

		/* Clear the GlobalID field while preserving other bits :
		 *    (Handle &~ S_GLOBAL_ID_MASK)
		 *
		 * No need to shift the value since the field is already from bits 0 - 31.
		 * Mask the value to prevent higher-order bits from being modified.
		 * Set the new GlobalID bits by bitwise OR'ing. */
		m_Handle = (m_Handle & ~S_GlobalID_Mask) |
			(globalID & S_GlobalID_Mask);
	}

	[[nodiscard]] AssetID AssetID::Registered(AssetType type, uint32_t globalID) noexcept
	{
		AssetID handle(type, globalID);
		handle.SetRegistered(true);
		return handle;
	}

	AssetID::View::View(AssetID id) noexcept
		: IsRegistered(id.IsRegistered()),
		  Type(id.Type()),
		  GlobalID(id.GlobalID())
	{
	}
}