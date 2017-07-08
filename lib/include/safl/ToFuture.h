/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "Future.h"

// Std includes:
#include <tuple>

namespace safl
{

namespace detail
{

template <typename CbType>
struct FuturizeWrapper
{
};

template<typename Func, typename... Args, typename... CbArgs>
auto futurize(Func func, FuturizeWrapper<std::function<void(CbArgs...)>>, Args&&... args)
{
    Promise<std::tuple<std::remove_reference_t<CbArgs>...>> promise;
    (*func)(args..., [promise](CbArgs... cbArgs) mutable
    {
        promise.setValue(std::make_tuple(cbArgs...));
    });
    return promise.future();
}

} // namespace detail

template<typename R, typename... ArgsWithCb, typename... Args,
         typename CbType = std::tuple_element_t<sizeof...(ArgsWithCb)-1,
                                                std::tuple<std::remove_reference_t<ArgsWithCb>...>>>
auto futurize(R(*func)(ArgsWithCb...), Args&&... args)
{
    return detail::futurize(func, detail::FuturizeWrapper<CbType>{}, std::forward<Args>(args)...);
}

} // namespace safl
