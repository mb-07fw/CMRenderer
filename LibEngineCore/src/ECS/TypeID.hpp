#pragma once

#include "Types.hpp"

#include <cstdint>

namespace CMEngine::ECS
{
	/* A simple POD that contains an ID.
	 * Relies on TypeWrangler to generate an ID for any given type. */
	struct TypeID
	{
		TypeID() = default;
		~TypeID() = default;

		inline explicit TypeID(int32_t ID) noexcept
			: ID(ID)
		{
		}

		inline [[nodiscard]] bool operator==(TypeID other) const noexcept
		{
			return ID == other.ID;
		}

		inline operator int32_t() const noexcept
		{
			return ID;
		}

		constexpr static int32_t S_Invalid_ID = -1;
		int32_t ID = S_Invalid_ID;
	};

	/* A class that uses static memory to generate a unique ID (technically an index)
	 * for every type provided to GetTypeID().
	 *
	 * Note to self : This is currently not thread safe due to how static memory works. */
	class TypeWrangler
	{
	public:
		/* For each template - instantiation of Ty, a different static ID is retrieved.
		 *   (a non-qualified type is preferred so that different instantiations aren't
		 *      generated for different type qualifiers, ex. int, const int) 
		 */
		template <NonQualified Ty>
		inline static [[nodiscard]] TypeID GetTypeID() noexcept;

		template <NonQualified... Types>
		inline static [[nodiscard]] std::array<TypeID, sizeof...(Types)> GetTypeIDs() noexcept;
	private:
		/* Returns a different ID each call, as the ID increments every call. */
		inline static [[nodiscard]] TypeID NextTypeID() noexcept;

		inline static TypeID s_NextID = {};
	};

	template <NonQualified Ty>
	inline [[nodiscard]] TypeID TypeWrangler::GetTypeID() noexcept
	{
		/* For each call of GetTypeID :
		 *   The compiler checks if the static s_TypeID variable has already been initialized for that particular type T.
		 *   If it has, it returns the cached value.
		 *   If it hasn't, it initializes it by calling NextTypeID() exactly once. */
		static TypeID s_TypeID = NextTypeID();
		return s_TypeID;
	}

	inline [[nodiscard]] TypeID TypeWrangler::NextTypeID() noexcept
	{
		return TypeID(++s_NextID.ID);
	}

	template <NonQualified... Types>
	inline static [[nodiscard]] std::array<TypeID, sizeof...(Types)> TypeWrangler::GetTypeIDs() noexcept
	{
		return std::array<TypeID, sizeof...(Types)>{ GetTypeID<Types>()... };
	}

	template <NonQualified Ty>
	inline static [[nodiscard]] TypeID GetTypeID() noexcept
	{
		return TypeWrangler::GetTypeID<Ty>();
	}
}