#pragma once

namespace CMCommon
{
	struct CMECSTypeID {
		inline explicit CMECSTypeID(size_t ID) noexcept
			: ID(ID)
		{
		}

		bool operator==(const CMECSTypeID& other) const noexcept {
			return ID == other.ID;
		}

		inline operator size_t() const noexcept
		{
			return ID;
		}

		CMECSTypeID() = default;
		~CMECSTypeID() = default;

		size_t ID = 0;
	};

	// Note to self : This is currently not thread safe, due to static memory works.
	class CMECSTypeWrangler
	{
	public:
		// For each template-instantiation of Ty, a different static ID is retrieved. 
		template <typename Ty>
		inline static CMECSTypeID GetTypeID() noexcept;
	private:
		// Returns a different ID each call, as the ID increments every call.
		inline static CMECSTypeID NextTypeID() noexcept;

		inline static CMECSTypeID s_NextID = {};
	};

	template <typename Ty>
	inline CMECSTypeID CMECSTypeWrangler::GetTypeID() noexcept
	{
		/* For each call of GetTypeID : 
		 * 
		 * The compiler checks if the static s_TypeID variable has already been initialized for that particular type T.
		 * If it has, it returns the cached value.
		 * If it hasn't, it initializes it by calling NextTypeID() exactly once.
		 */
		static CMECSTypeID s_TypeID = NextTypeID();
		return s_TypeID;
	}

	inline CMECSTypeID CMECSTypeWrangler::NextTypeID() noexcept
	{
		return CMECSTypeID(s_NextID.ID++);
	}
}

namespace std
{
	template <>
	struct hash<CMCommon::CMECSTypeID> {
		size_t operator()(const CMCommon::CMECSTypeID& typeID) const noexcept {
			return hash<size_t>{}(typeID.ID);
		}
	};
}