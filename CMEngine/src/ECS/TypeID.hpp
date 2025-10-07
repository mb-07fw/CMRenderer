#pragma once

#include <cstdint>

namespace CMEngine::ECS
{
	/* A simple POD that contains an ID.
	 * Relies on CMTypeWrangler to generate an ID for any given type. */
	struct TypeID
	{
		inline explicit TypeID(int32_t ID) noexcept
			: ID(ID)
		{
		}

		bool operator==(TypeID other) const noexcept
		{
			return ID == other.ID;
		}

		inline operator int32_t() const noexcept
		{
			return ID;
		}

		TypeID() = default;
		~TypeID() = default;

		constexpr static int32_t S_INVALID_ID = -1;
		int32_t ID = S_INVALID_ID;
	};

	/* A class that uses static memory to generate a unique ID (technically an index)
	 * for every type provided to GetTypeID().
	 *
	 * Note to self : This is currently not thread safe due to how static memory works. */
	class TypeWrangler
	{
	public:
		/* For each template - instantiation of Ty, a different static ID is retrieved. */
		template <typename Ty>
		inline static TypeID GetTypeID() noexcept;
	private:
		/* Returns a different ID each call, as the ID increments every call. */
		inline static TypeID NextTypeID() noexcept;

		inline static TypeID s_NextID = {};
	};

	template <typename Ty>
	inline TypeID TypeWrangler::GetTypeID() noexcept
	{
		/* For each call of GetTypeID :
		 *   The compiler checks if the static s_TypeID variable has already been initialized for that particular type T.
		 *   If it has, it returns the cached value.
		 *   If it hasn't, it initializes it by calling NextTypeID() exactly once. */
		static TypeID s_TypeID = NextTypeID();
		return s_TypeID;
	}

	inline TypeID TypeWrangler::NextTypeID() noexcept
	{
		return TypeID(++s_NextID.ID);
	}
}