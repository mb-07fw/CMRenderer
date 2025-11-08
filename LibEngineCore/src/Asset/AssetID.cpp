#include "PCH.hpp"
#include "Asset/AssetID.hpp"
#include "Macros.hpp"

namespace CMEngine::Asset
{
	AssetID::AssetID(AssetType type, uint32_t globalID) noexcept
	{
		SetType(type);
		SetGlobalID(globalID);
	}

	[[nodiscard]] bool AssetID::IsRegistered() const noexcept
	{
		/* Mask out the value in the IsRegistered field (bit 63).
		 * Shift it down from bit 63 to bit 0 to produce a boolean value.
		 */
		return static_cast<bool>((m_Handle & S_IS_REGISTERED_MASK) >> S_IS_REGISTERED_SHIFT);
	}

	[[nodiscard]] AssetType AssetID::Type() const noexcept
	{
		/* Mask out the value in the AssetType field (bits 56 - 62).
		 * Shift it down to get the original enum value.
		 */
		return static_cast<AssetType>((m_Handle & S_ASSET_TYPE_MASK) >> S_ASSET_TYPE_SHIFT);
	}

	[[nodiscard]] uint32_t AssetID::GlobalID() const noexcept
	{
		/* Mask out the value in the GlobalID field (bits 0 - 31).
		 * No shift needed, as the GlobalID field is already aligned at bit 0.
		 */
		return m_Handle & S_GLOBAL_ID_MASK;
	}

	[[nodiscard]] AssetIDView AssetID::AsView() const noexcept
	{
		return AssetIDView(*this);
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
		 * Set the new IsRegistered bit by bitwise OR'ing.
		 */
		m_Handle = (m_Handle & ~S_IS_REGISTERED_MASK) |
			(static_cast<uint32_t>(isRegistered) << S_IS_REGISTERED_SHIFT);
	}

	void AssetID::SetType(AssetType type) noexcept
	{
		CM_ENGINE_ASSERT(IsValidAssetType(type));

		/* Clear the AssetType field :
		 *    (Handle &~ S_ASSET_TYPE_MASK)
		 *
		 * Cast type to a uint32_t for correct bitwise shifting.
		 * Shift the value up to its position in the field (bits 56 - 62).
		 * Mask the shifted value so that only the 7 bits that correspond to the type field remain � all higher-order bits are zeroed.
		 * Set the new AssetType bits by bitwise OR'ing.
		 */
		m_Handle = (m_Handle & ~S_ASSET_TYPE_MASK) |
			((static_cast<uint32_t>(type) << S_ASSET_TYPE_SHIFT) & S_ASSET_TYPE_MASK);
	}

	void AssetID::SetGlobalID(uint32_t globalID) noexcept
	{
		CM_ENGINE_ASSERT(globalID < S_GLOBAL_ID_MAXIMUM);

		/* Clear the GlobalID field while preserving other bits :
		 *    (Handle &~ S_GLOBAL_ID_MASK)
		 *
		 * No need to shift the value since the field is already from bits 0 - 31.
		 * Mask the value to prevent higher-order bits from being modified.
		 * Set the new GlobalID bits by bitwise OR'ing.
		 */
		m_Handle = (m_Handle & ~S_GLOBAL_ID_MASK) |
			(globalID & S_GLOBAL_ID_MASK);
	}

	[[nodiscard]] AssetID AssetID::Registered(AssetType type, uint32_t globalID) noexcept
	{
		AssetID handle(type, globalID);
		handle.SetRegistered(true);
		return handle;
	}

	AssetIDView::AssetIDView(AssetID handle) noexcept
	{
		IsRegistered = handle.IsRegistered();
		Type = handle.Type();
		GlobalID = handle.GlobalID();
	}
}