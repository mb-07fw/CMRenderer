#pragma once

#include <memory> 
#include <functional>

#include "Common/Utility.hpp"

namespace CMEngine::Core::Asset
{
	enum class AssetType : uint8_t
	{
		INVALID,
		MESH,
		TEXTURE
	};

	inline constexpr [[nodiscard]] bool IsValidAssetType(AssetType type) noexcept
	{
		switch (type)
		{
		case AssetType::MESH: [[fallthrough]];
		case AssetType::TEXTURE:
			return true;
		case AssetType::INVALID: [[fallthrough]];
		default:
			return false;
		}
	}

	using RawAssetHandle = uint32_t;

	/* Forward declare here for ToView(). */
	class AssetHandleView;

	struct AssetHandle
	{
		friend class AssetManager;

		AssetHandle(AssetType type, uint32_t globalID) noexcept;
		AssetHandle() = default;
		~AssetHandle() = default;

		[[nodiscard]] bool IsRegistered() const noexcept;
		[[nodiscard]] AssetType Type() const noexcept;
		[[nodiscard]] uint32_t GlobalID() const noexcept;
		[[nodiscard]] AssetHandleView AsView() const noexcept;

		inline [[nodiscard]] RawAssetHandle RawHandle() const noexcept { return Handle; }
		inline [[nodiscard]] bool IsValid() const noexcept { return Handle != S_INVALID_HANDLE; }
		inline static [[nodiscard]] AssetHandle Invalid() noexcept { return AssetHandle{}; }

		[[nodiscard]] bool operator==(AssetHandle other) const noexcept;
	private:
		void SetRegistered(bool isRegistered) noexcept;
		void SetType(AssetType type) noexcept;
		void SetGlobalID(uint32_t globalID) noexcept;

		static [[nodiscard]] AssetHandle Registered(AssetType type, uint32_t globalID) noexcept;
	private:
		RawAssetHandle Handle = 0;
	public:
		static constexpr uint32_t S_INVALID_HANDLE = 0;

		/*
		 * Because I'll forget later --
		 *
		 * Shift variables describe the start position of the bit field,
		 * Masks isolate bit values by containing a value with only a certain set of bits enabled.
		 *
		 * Love this; see behavior of bitwise operations here : https://www.geeksforgeeks.org/cpp-bitwise-operators/
		 */

		inline static constexpr uint32_t S_GLOBAL_ID_BITS = 24;
		inline static constexpr uint32_t S_ASSET_TYPE_BITS = 7;
		inline static constexpr uint32_t S_IS_REGISTERED_BITS = 1;

		inline static constexpr uint32_t S_MAXIMUM_GLOBAL_IDS = Common::Utility::ToPower<uint32_t>(2, 8 * 3);
		inline static constexpr uint32_t S_MAXIMUM_ASSET_TYPES = Common::Utility::ToPower<uint32_t>(2, 7);

		static constexpr uint32_t S_GLOBAL_ID_MAXIMUM = S_MAXIMUM_GLOBAL_IDS - 1;

		/* Start positions of all bit-fields ... */
		static constexpr uint32_t S_GLOBAL_ID_SHIFT = 0;
		static constexpr uint32_t S_ASSET_TYPE_SHIFT = S_GLOBAL_ID_BITS;
		static constexpr uint32_t S_IS_REGISTERED_SHIFT = S_ASSET_TYPE_SHIFT + S_ASSET_TYPE_BITS;

		/* uint64_t Asset Handle layout :
		 *
		 * 31			31 30	    24 23			  0
		 * +--------------+-----------+---------------+
		 * | IsRegistered | AssetType | GlobalAssetID |
		 * +--------------+-----------+---------------+
		 *      1 bit		  7 bits	     24 bits
		 *
		 */

		 /* Shift 1 (unsigned 32 bit) left by 24 bits :
		  *
		  *		00000000 00000000 00000000 00000001
		  *						to :
		  *		00000001 00000000 00000000 00000000
		  *
		  * Subtract 1 to mask the lower 24 bits :
		  *
		  *	 NOTE : Subtracting 1 in binary means you're flipping the
		  *			lowest 1 bit to 0, and all trailing 0s to 1s until
		  *			the first 1 (carry/borrow effect).
		  *
		  *		00000001 00000000 00000000 00000000
		  *						to :
		  *		00000000 11111111 11111111 11111111
		  *
		  * No need to shift; bit-field already starts at bit 0.
		  */
		inline static constexpr uint32_t S_GLOBAL_ID_MASK = ((static_cast<uint32_t>(1) << S_GLOBAL_ID_BITS) - 1);

		/* Shift 1 (unsigned 32 bit) left by 7 bits :
		 *
		 *		00000000 00000000 00000000 00000001
		 *						to :
		 *		00000000 00000000 00000000 10000000
		 *
		 * Subtract 1 to mask the lower 7 bits :
		 *
		 *	 NOTE : Subtracting 1 in binary means you're flipping the
		 *			lowest 1 bit to 0, and all trailing 0s to 1s until
		 *			the first 1 (carry/borrow effect).
		 *
		 *		00000000 00000000 00000000 10000000
		 *						to :
		 *		00000000 00000000 00000000 01111111
		 *
		 * Shift to position of AssetType field (bit 24 - bit 30, shift up by 24 bits, or 3 bytes) :
		 *
		 *		00000000 00000000 00000000 01111111
		 *						to :
		 *      01111111 00000000 00000000 00000000
		 */
		inline static constexpr uint32_t S_ASSET_TYPE_MASK = ((static_cast<uint32_t>(1) << S_ASSET_TYPE_BITS) - 1)
			<< S_ASSET_TYPE_SHIFT;

		/* Shift 1 (unsigned 32 bit) left by 1 bit :
		 *
		 *		00000000 00000000 00000000 00000001
		 *						to :
		 *		00000000 00000000 00000000 00000010
		 *
		 * Subtract 1 to mask the lower bit :
		 *
		 *	 NOTE : Subtracting 1 in binary means you're flipping the
		 *			lowest 1 bit to 0, and all trailing 0s to 1s until
		 *			the first 1 (carry/borrow effect).
		 *
		 *		00000000 00000000 00000000 00000010
		 *						to :
		 *		00000000 00000000 00000000 00000001
		 *
		 * Shift to position of IsRegistered field (bit 31, shift up by 32 bits) :
		 *
		 *		00000000 00000000 00000000 00000001
		 *						to :
		 *      10000000 00000000 00000000 00000000
		 */
		inline static constexpr uint32_t S_IS_REGISTERED_MASK = ((static_cast<uint32_t>(1) << S_IS_REGISTERED_BITS) - 1)
			<< S_IS_REGISTERED_SHIFT;
	};

	class AssetHandleView
	{
	public:
		AssetHandleView(AssetHandle handle) noexcept;
		~AssetHandleView() = default;

		bool IsRegistered = false;
		AssetType Type = AssetType::INVALID;
		uint32_t GlobalID = 0;
	};
}

namespace std
{
	template <>
	struct hash<CMEngine::Core::Asset::AssetHandle>
	{
		size_t operator()(CMEngine::Core::Asset::AssetHandle handle) const noexcept
		{
			return hash<size_t>{}(handle.RawHandle());
		}
	};
}