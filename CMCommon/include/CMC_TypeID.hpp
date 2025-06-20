#pragma once

namespace CMCommon
{
	/* A simple POD that contains an ID. 
	 * Relies on CMTypeWrangler to generate an ID for any given type.
	 */
	struct CMTypeID
	{
		inline explicit CMTypeID(size_t ID) noexcept
			: ID(ID)
		{
		}

		bool operator==(CMTypeID other) const noexcept {
			return ID == other.ID;
		}

		inline operator size_t() const noexcept
		{
			return ID;
		}

		CMTypeID() = default;
		~CMTypeID() = default;

		size_t ID = 0;
	};

	/* A class that uses static memory to generate a unique ID (technically an index)
	 * for every type provided to GetTypeID(). 
	 * 
	 * Note to self : This is currently not thread safe, due to static memory works.
	 */
	class CMTypeWrangler
	{
	public:
		// For each template-instantiation of Ty, a different static ID is retrieved. 
		template <typename Ty>
		inline static CMTypeID GetTypeID() noexcept;
	private:
		// Returns a different ID each call, as the ID increments every call.
		inline static CMTypeID NextTypeID() noexcept;

		inline static CMTypeID s_NextID = {};
	};

	template <typename Ty>
	inline CMTypeID CMTypeWrangler::GetTypeID() noexcept
	{
		/* For each call of GetTypeID : 
		 * 
		 * The compiler checks if the static s_TypeID variable has already been initialized for that particular type T.
		 * If it has, it returns the cached value.
		 * If it hasn't, it initializes it by calling NextTypeID() exactly once.
		 */
		static CMTypeID s_TypeID = NextTypeID();
		return s_TypeID;
	}

	inline CMTypeID CMTypeWrangler::NextTypeID() noexcept
	{
		return CMTypeID(s_NextID.ID++);
	}
}

namespace std
{
	template <>
	struct hash<CMCommon::CMTypeID>
	{
		size_t operator()(CMCommon::CMTypeID typeID) const noexcept
		{
			return hash<size_t>{}(typeID.ID);
		}
	};
}