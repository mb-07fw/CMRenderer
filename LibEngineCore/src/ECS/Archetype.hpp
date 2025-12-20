#pragma once

#include "ECS/Entity.hpp"
#include "ECS/TypeID.hpp"
#include "Utility.hpp"
#include "Types.hpp"
#include "Macros.hpp"

#include <tuple>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <iostream>

namespace CMEngine::ECS
{
#pragma region Traits
	template <typename... Types>
	class Archetype;

	template <typename Ty>
	struct IsArchetypeType : std::false_type {};

	template <typename... Types>
	struct IsArchetypeType<Archetype<Types...>> : std::true_type {};

	template <typename Ty>
	inline constexpr bool IsArchetypeType_v = IsArchetypeType<Ty>::value;

	template <typename Ty>
	concept ArchetypeType = IsArchetypeType_v<Ty>;

	template <typename TL>
	struct ListToArchetype;

	template<typename... Types>
	struct ListToArchetype<TypeList<Types...>> {
		using Type = Archetype<Types...>;
	};

	template <typename Remove, typename... Types>
	using ArchetypeRemoved = ListToArchetype<Remove_t<Remove, Types...>>::Type;

	template <typename Add, typename... Types>
	using ArchetypeAdded = ListToArchetype<Add_t<Add, Types...>>::Type;

	template <typename TypesTuple, typename ParamsTuple, std::size_t... I>
	inline consteval void CheckForMismatchingTypes(std::index_sequence<I...>)
	{
		static_assert(
			(std::is_same_v<std::tuple_element_t<I, TypesTuple>, typename std::tuple_element_t<I, ParamsTuple>::ConstructTy> && ...),
			"Type mismatch between target type and ParamsRef::ConstructTy.");
	}
#pragma endregion

	struct ArchetypeID
	{
		ArchetypeID() = default;
		~ArchetypeID() = default;

		inline static [[nodiscard]] ArchetypeID Invalid() noexcept { return ArchetypeID{}; }

		inline [[nodiscard]] bool IsInvalid() const noexcept { return Hash == 0 && TypeBitset == 0; }

		inline [[nodiscard]] bool IsTypeSet(TypeID id) const noexcept
		{
			constexpr size_t NumBits = sizeof(size_t) * 8;
			CM_ENGINE_ASSERT(id.ID < NumBits);
			return TypeBitset & (static_cast<size_t>(1) << id.ID);
		}

		inline void SetType(TypeID id) noexcept
		{
			constexpr size_t NumBits = sizeof(size_t) * 8;
			CM_ENGINE_ASSERT(id.ID < NumBits);
			TypeBitset |= static_cast<size_t>(1) << id.ID;
		}
		
		inline [[nodiscard]] bool operator==(const ArchetypeID& other) const noexcept
		{
			return Hash == other.Hash &&
				TypeBitset == other.TypeBitset;
		}

		size_t Hash = 0;
		size_t TypeBitset = 0; /* [0, 63] ID's... */
	};

	template <typename... Types>
	inline [[nodiscard]] ArchetypeID GetArchetypeID() noexcept
	{
		ArchetypeID id = {};

		std::array<TypeID, sizeof...(Types)> typeIDs = TypeWrangler::GetTypeIDs<Types...>();

		/* Get unique hash from all TypeID's, and set bit of each ID. */
		for (TypeID typeID : typeIDs)
		{
			HashCombine(id.Hash, typeID.ID);
			id.TypeBitset |= static_cast<size_t>(1) << typeID.ID;
		}

		return id;
	}

	class IColumn
	{
	public:
		IColumn() = default;
		virtual ~IColumn() = default;
	private:
	};

	template <typename Ty>
	class Column : public IColumn
	{
	public:
		Column() = default;
		~Column() = default;

		inline [[nodiscard]] Ty& At(size_t index) noexcept
		{
			CM_ENGINE_ASSERT(index < m_Elems.size());
			return m_Elems[index];
		}

		inline [[nodiscard]] const Ty& At(size_t index) const noexcept
		{
			CM_ENGINE_ASSERT(index < m_Elems.size());
			return m_Elems[index];
		}

		inline [[nodiscard]] Ty& Back() noexcept
		{
			CM_ENGINE_ASSERT(Size() > 0);
			return m_Elems.back();
		}

		inline [[nodiscard]] const Ty& Back() const noexcept
		{
			CM_ENGINE_ASSERT(Size() > 0);
			return m_Elems.back();
		}

		inline void PushBack(const Ty& value) noexcept
		{
			m_Elems.push_back(value);
		}

		template <typename... Args>
		inline void EmplaceBack(Args&&... args) noexcept
		{
			m_Elems.emplace_back(std::forward<Args>(args)...);
		}

		inline void Destroy(size_t index) noexcept
		{
			CM_ENGINE_ASSERT(index < m_Elems.size());

			/* Swap element at index with last... */
			if (Size() > 1)
				m_Elems[index] = std::move(Back());

			m_Elems.pop_back();
		}

		inline [[nodiscard]] size_t Size() const noexcept { return m_Elems.size(); }
	private:
		std::vector<Ty> m_Elems;
	};

	class IArchetype
	{
	public:
		IArchetype() = default;
		virtual ~IArchetype() = default;

		virtual [[nodiscard]] ArchetypeID ID() const noexcept = 0;
		virtual bool DestroyRow(size_t index) noexcept = 0;
	};

	template <typename... Types>
	class Archetype : public IArchetype
	{
	public:
		using TLTypes = TypeList<Types...>;
		static constexpr size_t S_NumTypes = sizeof...(Types);

		inline Archetype() noexcept;
		~Archetype() = default;

		inline virtual bool DestroyRow(size_t index) noexcept override;

		template <typename... LastArgs>
		inline void TransferBack(
			ArchetypeRemoved<Last_t<Types...>, Types...>& oldArchetype,
			size_t oldIndex,
			LastArgs&&... args
		) noexcept;

		template <typename... ParamsTypes>
		inline void EmplaceBack(ParamsTypes&&... params) noexcept;

		template <typename Ty>
		inline static [[nodiscard]] std::unique_ptr<ArchetypeAdded<Ty, Types...>> Added() noexcept;

		template <typename Ty>
			requires IsInPack<Ty, Types...>
		inline static [[nodiscard]] std::unique_ptr<ArchetypeRemoved<Ty, Types...>> Removed() noexcept;

		inline virtual [[nodiscard]] ArchetypeID ID() const noexcept override { return m_ID; }

		inline [[nodiscard]] size_t Size() const noexcept { return std::get<0>(m_Columns).Size(); }

		template <typename Ty>
			requires IsInPack<Ty, Types...>
		inline [[nodiscard]] Ty& Get(size_t index) noexcept { return GetColumn<Ty>().At(index); }

		template <typename Ty>
			requires IsInPack<Ty, Types...>
		inline [[nodiscard]] const Ty& Get(size_t index) const noexcept { return GetColumn<Ty>().At(index); }

		template <typename Ty>
			requires IsInPack<Ty, Types...>
		inline [[nodiscard]] Column<Ty>& GetColumn() noexcept { return std::get<Column<Ty>>(m_Columns); }

		template <typename Ty>
			requires IsInPack<Ty, Types...>
		inline [[nodiscard]] const Column<Ty>& GetColumn() const noexcept { return std::get<Column<Ty>>(m_Columns); }
	private:
		template <typename... ParamsTypes, size_t... Indices>
		inline void EmplaceBackImpl(std::index_sequence<Indices...>, ParamsTypes&&... params) noexcept;

		template <typename... PreviousTypes>
		inline void MovePrevious(Archetype<PreviousTypes...>& oldArchetype, size_t index) noexcept;
	private:
		ArchetypeID m_ID = GetArchetypeID<Types...>();

		/* TODO: Change to Column<Types>... */
		std::tuple<Column<Types>...> m_Columns;
	};

	using ArchetypeRemovedTest = decltype(Archetype<float, int, char>::Removed<int>());
	static_assert(std::is_same_v<ArchetypeRemovedTest, std::unique_ptr<Archetype<float, char>>>, "Failed to pass sanity check for Archetype<>::Removed<>()...");

	using ArchetypeRemoveTest = ArchetypeRemoved<int, char, float, int>;
	static_assert(std::is_same_v<ArchetypeRemoveTest, Archetype<char, float>>, "Failed to pass sanity check for ArchitypeRemove...");

	using ArchetypeAddTest = ArchetypeAdded<int, char, float>;
	static_assert(std::is_same_v<ArchetypeAddTest, Archetype<char, float, int>>, "Failed to pass sanity check for ArchitypeAdd...");

	template <typename... Types>
	inline Archetype<Types...>::Archetype() noexcept
	{
	}

	template <typename... Types>
	inline bool Archetype<Types...>::DestroyRow(size_t index) noexcept
	{
		if (index >= Size())
			return false;

		(GetColumn<Types>().Destroy(index), ...);
		return true;
	}

	template <typename... Types>
	template <typename... LastArgs>
	inline void Archetype<Types...>::TransferBack(
		ArchetypeRemoved<Last_t<Types...>, Types...>& oldArchetype,
		size_t oldIndex,
		LastArgs&&... lastArgs
	) noexcept
	{
		MovePrevious(oldArchetype, oldIndex);
		GetColumn<Last_t<Types...>>().EmplaceBack(std::forward<LastArgs>(lastArgs)...);
	}

	template <typename... Types>
	template <typename... ParamsTypes>
	inline void Archetype<Types...>::EmplaceBack(ParamsTypes&&... params) noexcept
	{
		static_assert(
			AllParams_v<ParamsTypes...>,
			"Each type of each argument provided should be an instantiation of ParamsRef..."
		);

		static_assert(
			sizeof...(Types) == sizeof...(ParamsTypes),
			"The number of Params should match the number of types present."
		);

		CheckForMismatchingTypes<std::tuple<Types...>, std::tuple<ParamsTypes...>>(
			std::index_sequence_for<Types...>{}
		);
		
		/* Forward each Params instance with a respective index for
		 *   parallel tuple expansion of params and columns. */
		EmplaceBackImpl(
			std::index_sequence_for<Types...>{},
			std::forward<ParamsTypes>(params)...
		);
	}

	template <typename... Types>
	template <typename Ty>
	inline [[nodiscard]] std::unique_ptr<ArchetypeAdded<Ty, Types...>> Archetype<Types...>::Added() noexcept
	{
		return nullptr;
	}

	template <typename... Types>
	template <typename Ty>
		requires IsInPack<Ty, Types...>
	inline [[nodiscard]] std::unique_ptr<ArchetypeRemoved<Ty, Types...>> Archetype<Types...>::Removed() noexcept
	{
		return nullptr;
	}

	template <typename... Types>
	template <typename... ParamsTypes, size_t... Indices>
	inline void Archetype<Types...>::EmplaceBackImpl(std::index_sequence<Indices...>, ParamsTypes&&... params) noexcept
	{
		/* For each index in the parameter pack, the corresponding Column and Params are retrieved.
		 *
		 * Fold expression resolves std::get for each index, meaning each Params instance is
		 *   correctly forwarded into it's respective Column. */
		(
			std::apply(
				[&](auto&&... ctorArgs) {
					using Ty = std::tuple_element_t<Indices, std::tuple<Types...>>;
					std::get<Indices>(m_Columns).EmplaceBack(
						std::forward<decltype(ctorArgs)>(ctorArgs)...
					);
				}, 
				std::forward<ParamsTypes>(params).StoredParams
			),
			...
		);
	}

	template <typename... Types>
	template <typename... PreviousTypes>
	inline void Archetype<Types...>::MovePrevious(Archetype<PreviousTypes...>& oldArchetype, size_t index) noexcept
	{
		(
			GetColumn<PreviousTypes>().EmplaceBack(
				std::move(oldArchetype.Get<PreviousTypes>(index))
			),
			...
		);

		oldArchetype.DestroyRow(index);
	}
}

namespace std
{
	template <>
	struct hash<CMEngine::ECS::ArchetypeID>
	{
		inline [[nodiscard]] size_t operator()(CMEngine::ECS::ArchetypeID id) const noexcept
		{
			return id.Hash;
		}
	};
}