#pragma once

#include <type_traits>
#include <concepts>

template <typename Ty>
inline constexpr bool IsPtr_v = std::is_pointer_v<Ty>;

template <typename Ty>
concept Ptr = std::is_pointer_v<Ty>;

template <typename To, typename From>
inline constexpr [[nodiscard]] To Cast(From from) noexcept
{
    return static_cast<To>(from);
}

template <typename To, typename From>
inline constexpr [[nodiscard]] To* CastPtr(From* pFrom) noexcept
{
    return static_cast<To*>(pFrom);
}

template <Ptr To, Ptr From>
    requires std::is_base_of_v<
        std::remove_pointer_t<std::remove_cvref_t<From>>,
        std::remove_pointer_t<std::remove_cvref_t<To>>>
inline [[nodiscard]] To TryCast(From pFrom) noexcept
{
    return dynamic_cast<To>(pFrom);
}

template <typename To, typename From>
inline [[nodiscard]] To Reinterpret(From pFrom) noexcept
{
    return reinterpret_cast<To>(pFrom);
}