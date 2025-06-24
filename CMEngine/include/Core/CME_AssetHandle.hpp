#pragma once

#include "CMC_Utility.hpp"

#include <memory> 
#include <functional>

namespace CMEngine::Asset
{
	enum class CMAssetType : uint8_t
	{
		INVALID,
		MESH,
		TEXTURE
	};

	inline constexpr [[nodiscard]] bool IsValidAssetType(CMAssetType type) noexcept
	{
		switch (type)
		{
		case CMAssetType::MESH: [[fallthrough]];
		case CMAssetType::TEXTURE:
			return true;
		case CMAssetType::INVALID: [[fallthrough]];
		default:
			return false;
		}
	}

	using CMRawAssetHandle = uint32_t;

	/* Forward declare here for ToView(). */
	class CMAssetHandleView;

	struct CMAssetHandle
	{
		friend class CMAssetManager;

		CMAssetHandle(CMAssetType type, uint32_t globalID) noexcept;
		CMAssetHandle() = default;
		~CMAssetHandle() = default;

		[[nodiscard]] bool IsRegistered() const noexcept;
		[[nodiscard]] CMAssetType Type() const noexcept;
		[[nodiscard]] uint32_t GlobalID() const noexcept;
		[[nodiscard]] CMAssetHandleView AsView() const noexcept;

		inline [[nodiscard]] CMRawAssetHandle RawHandle() const noexcept { return Handle; }
		inline [[nodiscard]] bool IsValid() const noexcept { return Handle != S_INVALID_HANDLE; }
		inline static [[nodiscard]] CMAssetHandle Invalid() noexcept { return CMAssetHandle{}; }

		[[nodiscard]] bool operator==(CMAssetHandle other) const noexcept;
	private:
		void SetRegistered(bool isRegistered) noexcept;
		void SetType(CMAssetType type) noexcept;
		void SetGlobalID(uint32_t globalID) noexcept;

		static [[nodiscard]] CMAssetHandle Registered(CMAssetType type, uint32_t globalID) noexcept;
	private:
		CMRawAssetHandle Handle = 0;
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

		inline static constexpr uint32_t S_MAXIMUM_GLOBAL_IDS = CMCommon::Utility::ToPower<uint32_t>(2, 8 * 3);
		inline static constexpr uint32_t S_MAXIMUM_ASSET_TYPES = CMCommon::Utility::ToPower<uint32_t>(2, 7);

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

	class CMAssetHandleView
	{
	public:
		CMAssetHandleView(CMAssetHandle handle) noexcept;
		~CMAssetHandleView() = default;

		bool IsRegistered = false;
		CMAssetType Type = CMAssetType::INVALID;
		uint32_t GlobalID = 0;
	};
}

namespace std
{
	template <>
	struct hash<CMEngine::Asset::CMAssetHandle>
	{
		size_t operator()(CMEngine::Asset::CMAssetHandle handle) const noexcept
		{
			return hash<size_t>{}(handle.RawHandle());
		}
	};
}