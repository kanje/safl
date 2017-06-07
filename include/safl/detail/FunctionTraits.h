/*
 * This file is a part of Standalone Future Library (safl).
 */

#pragma once

// Std includes:
#include <type_traits>
#include <tuple>

namespace safl
{
namespace detail
{

namespace detail // even more detail
{

template<typename tReturnType, typename... tArgs>
struct FunctionTraits
{
    using ReturnType = tReturnType;
    using NrArgs = std::integral_constant<std::size_t, sizeof...(tArgs)>;

    template <std::size_t idx>
    using Arg = typename std::remove_cv_t<
                             std::remove_reference_t<
                                 std::tuple_element_t<idx, std::tuple<tArgs...>>>>;

    using FirstArg = Arg<0>;
};

template<typename tReturnType>
struct FunctionTraits<tReturnType>
{
    using ReturnType = tReturnType;
    using NrArgs = std::integral_constant<std::size_t, 0>;
};

} // namespace detail

template<typename tFunc>
struct FunctionTraits
        : FunctionTraits<decltype(&std::remove_reference_t<tFunc>::operator())>
{
};

template<typename tReturnType, typename tClassType, typename... tArgs>
struct FunctionTraits<tReturnType(tClassType::*)(tArgs...) const>
        : detail::FunctionTraits<tReturnType, tArgs...>
{
};

template<typename tReturnType, typename tClassType, typename... tArgs>
struct FunctionTraits<tReturnType(tClassType::*)(tArgs...)>
        : detail::FunctionTraits<tReturnType, tArgs...>
{
};

template<typename tReturnType, typename tClassType, typename... tArgs>
struct FunctionTraits<tReturnType(tClassType::*&)(tArgs...)>
        : detail::FunctionTraits<tReturnType, tArgs...>
{
};

template<typename tReturnType, typename... tArgs>
struct FunctionTraits<tReturnType(*)(tArgs...)>
        : detail::FunctionTraits<tReturnType, tArgs...>
{
};

template<typename tReturnType, typename... tArgs>
struct FunctionTraits<tReturnType(&)(tArgs...)>
        : detail::FunctionTraits<tReturnType, tArgs...>
{
};

template<typename tReturnType, typename... tArgs>
struct FunctionTraits<tReturnType(tArgs...)>
        : detail::FunctionTraits<tReturnType, tArgs...>
{
};

} // namespace detail
} // namespace detail
