#pragma once

#include <cstdint>
#include <concepts>

namespace Engine::Asset
{
	template <std::unsigned_integral Ty, std::unsigned_integral ReturnTy = Ty>
	inline constexpr [[nodiscard]] ReturnTy ToPower(Ty value, Ty power) noexcept
	{
		if (power == 0)
			return 1u;

		ReturnTy newValue = 1;

		for (Ty i = 0; i < power; ++i)
			newValue *= value;

		return newValue;
	}

	enum class AssetType : uint8_t
	{
		Invalid,
		Model, /* Overarching owner of any subsequent meshes / materials. */
		Mesh,
		Material,
		Texture,
		Shader
	};

	inline constexpr [[nodiscard]] bool IsValidAssetType(AssetType type) noexcept
	{
		switch (type)
		{
		case AssetType::Model: [[fallthrough]];
		case AssetType::Mesh: [[fallthrough]];
		case AssetType::Material: [[fallthrough]];
		case AssetType::Texture: [[fallthrough]];
		case AssetType::Shader:
			return true;
		case AssetType::Invalid: [[fallthrough]];
		default:
			return false;
		}
	}

	using RawAssetID = uint32_t;

	struct AssetID
	{
		friend class AssetManager;

		/* Forward declare for ToView(). */
		struct View;

		AssetID(AssetType type, uint32_t globalID) noexcept;
		AssetID() = default;
		~AssetID() = default;

		[[nodiscard]] bool IsRegistered() const noexcept;
		[[nodiscard]] AssetType Type() const noexcept;
		[[nodiscard]] uint32_t GlobalID() const noexcept;

		[[nodiscard]] View ToView() const noexcept;

		inline [[nodiscard]] RawAssetID RawHandle() const noexcept { return m_Handle; }
		inline [[nodiscard]] bool IsValid() const noexcept { return m_Handle != S_InvalidHandle; }
		inline static [[nodiscard]] AssetID Invalid() noexcept { return AssetID{}; }

		[[nodiscard]] bool operator==(AssetID other) const noexcept;
		[[nodiscard]] bool operator<(AssetID other) const noexcept;
		inline [[nodiscard]] bool operator!() const noexcept { return !IsRegistered(); }
		inline operator bool() const noexcept { return IsRegistered(); }
	private:
		void SetRegistered(bool isRegistered) noexcept;
		void SetType(AssetType type) noexcept;
		void SetGlobalID(uint32_t globalID) noexcept;

		static [[nodiscard]] AssetID Registered(AssetType type, uint32_t globalID) noexcept;
	private:
		RawAssetID m_Handle = 0;
	public:
		struct View
		{
		public:
			View(AssetID id) noexcept;
			~View() = default;

			bool IsRegistered = false;
			AssetType Type = AssetType::Invalid;
			uint32_t GlobalID = 0;
		};

		static constexpr uint32_t S_InvalidHandle = 0;

		/* Because I'll forget later --
		 *
		 * Shift variables describe the start position of the bit field,
		 * Masks isolate bit values by containing a value with only a certain set of bits enabled.
		 *
		 * See behavior of bitwise operations here : https://www.geeksforgeeks.org/cpp-bitwise-operators/ */

		inline static constexpr uint32_t S_GlobalID_Bits = 24;
		inline static constexpr uint32_t S_AssetType_Bits = 7;
		inline static constexpr uint32_t S_IsRegistered_Bits = 1;

		inline static constexpr uint32_t S_Maximum_Global_IDs = ToPower<uint32_t>(2, 8 * 3);
		inline static constexpr uint32_t S_Maximum_Asset_Types = ToPower<uint32_t>(2, 7);

		static constexpr uint32_t S_GlobalID_Maximum = S_Maximum_Global_IDs - 1;

		/* Start positions of all bit-fields ... */
		static constexpr uint32_t S_GlobalID_Shift = 0;
		static constexpr uint32_t S_AssetType_Shift = S_GlobalID_Bits;
		static constexpr uint32_t S_IsRegistered_Shift = S_AssetType_Shift + S_AssetType_Bits;

		/* uint32_t Asset Handle layout :
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
		inline static constexpr uint32_t S_GlobalID_Mask = ((static_cast<uint32_t>(1) << S_GlobalID_Bits) - 1);

		/* Shift 1 (unsigned 32 bit) left by 7 bits :
		 *
		 *		00000000 00000000 00000000 00000001
		 *						to :
		 *		00000000 00000000 00000000 10000000
		 *
		 * Subtract 1 to mask the lower 7 bits :
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
		inline static constexpr uint32_t S_AssetType_Mask = ((static_cast<uint32_t>(1) << S_AssetType_Bits) - 1)
			<< S_AssetType_Shift;

		/* Shift 1 (unsigned 32 bit) left by 1 bit :
		 *
		 *		00000000 00000000 00000000 00000001
		 *						to :
		 *		00000000 00000000 00000000 00000010
		 *
		 * Subtract 1 to mask the lower bit :
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
		inline static constexpr uint32_t S_IsRegistered_Mask = ((static_cast<uint32_t>(1) << S_IsRegistered_Bits) - 1)
			<< S_IsRegistered_Shift;
	};
}

namespace std
{
	template <>
	struct hash<Engine::Asset::AssetID>
	{
		inline size_t operator()(Engine::Asset::AssetID id) const noexcept
		{
			return hash<size_t>{}(id.GlobalID());
		}
	};
}