#pragma once

#include "Common/Assert.hpp"

template <typename Ty>
class ViewBasic
{
public:
	inline ViewBasic(Ty* pData) noexcept
		: mP_Ptr(pData)
	{
	}

	ViewBasic() = default;
	~ViewBasic() = default;

	inline operator Ty* () noexcept { return mP_Ptr; }
	inline ViewBasic<Ty>& operator=(Ty* pData) noexcept { mP_Ptr = pData; return*this; }
	inline Ty* operator->() noexcept { return mP_Ptr; }

	inline [[nodiscard]] Ty* Raw() noexcept { return mP_Ptr; }
	inline [[nodiscard]] Ty& Ref();

	inline void Reset() noexcept { mP_Ptr = nullptr; }

	inline [[nodiscard]] bool NonNull() const noexcept { return mP_Ptr != nullptr; }
	inline [[nodiscard]] bool Null() const noexcept { return mP_Ptr == nullptr; }

	inline static [[nodiscard]] ViewBasic<Ty> NullView() noexcept { return ViewBasic<Ty>(); }
protected:
	Ty* mP_Ptr = nullptr;
};

template <typename Ty>
inline [[nodiscard]] Ty& ViewBasic<Ty>::Ref()
{
	ASSERT(NonNull(), "(View) Attempted to de-reference a null view.");
	return *Raw();
}

/* View represents a non-onwing pointer to a resource that is NOT meant
 *   to be deleted by the client. */
template <typename Ty>
using View = ViewBasic<Ty>;

/* ConstView represents a non-onwing pointer to a const resource that is NOT meant
 *   to be deleted by the client. */
template <typename Ty>
struct ConstView : public ViewBasic<const Ty>
{
	using Parent = ViewBasic<const Ty>;
	using Parent::Parent;

	inline ConstView(View<Ty> view) noexcept
		: Parent(view.Raw())
	{
	}

	inline ConstView& operator=(View<Ty> view) noexcept
	{
		Parent::mP_Ptr = view.Raw();
		return *this;
	}

	inline ConstView& operator=(Ty* pData) noexcept
	{
		Parent::mP_Ptr = pData;
		return *this;
	}

	/* Prevent nullptr from hitting both overloads... */
	inline ConstView& operator=(std::nullptr_t) noexcept
	{
		Parent::mP_Ptr = nullptr;
		return *this;
	}

	ConstView() = default;
	~ConstView() = default;
};

template <typename T>
inline [[nodiscard]] View<T> AsView(T& value) noexcept
{
	return View(&value);
}

template <typename T>
inline [[nodiscard]] ConstView<T> AsConstView(const T& value) noexcept
{
	return ConstView(&value);
}