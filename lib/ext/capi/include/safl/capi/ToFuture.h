/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include <safl/Future.h>

// Other includes:
#include <CommonAPI/CommonAPI.hpp>

namespace safl
{
namespace capi
{

/* CommonAPI 3 asynchronous methods look like this:
 *
 * virtual std::future<CommonAPI::CallStatus> doItAsync(
 *          const Arg &arg, ...,
 *          DoItAsyncCallback callback = nullptr,
 *          const CommonAPI::CallInfo *info = nullptr);
 */

namespace detail
{

struct CapiError
{
    CommonAPI::CallStatus callStatus;
};

template <typename tCbType>
struct CbWrapper
{
};

template<typename... tCbArgs>
struct CbArgsTraits
{
    using ValueType = std::tuple<std::remove_reference_t<tCbArgs>...>;
    static auto forward(tCbArgs&&... args)
    {
        return std::make_tuple(std::forward<tCbArgs>(args)...);
    }
};

template<typename tCbArg>
struct CbArgsTraits<tCbArg>
{
    using ValueType = std::remove_reference_t<tCbArg>;
    static auto forward(tCbArg &&arg)
    {
        return std::forward<tCbArg>(arg);
    }
};

template<typename tProxy, typename tFunc, typename... tArgs, typename... tCbArgs>
auto futurize(tProxy &proxy,
              tFunc func,
              CbWrapper<std::function<void(const CommonAPI::CallStatus&, tCbArgs...)>>,
              tArgs&&... args)
{
    using Traits = CbArgsTraits<tCbArgs...>;

    SharedPromise<typename Traits::ValueType> promise;
    (proxy.*func)(std::forward<tArgs>(args)..., [promise](const CommonAPI::CallStatus &callStatus, tCbArgs... cbArgs) mutable
    {
        if ( callStatus == CommonAPI::CallStatus::SUCCESS )
        {
            promise->setValue(Traits::forward(cbArgs...));
        }
        else
        {
            promise->setError(CapiError{callStatus});
        }
    }, nullptr);

    return promise->future();
}

} // namespace detail

using CapiError = detail::CapiError;

template<typename tProxy, typename... tCapiArgs, typename... tArgs>
auto futurize(tProxy &proxy, std::future<CommonAPI::CallStatus>(tProxy::*func)(tCapiArgs...), tArgs&&... args)
{
    static_assert(std::is_base_of<CommonAPI::Proxy, tProxy>::value,
                  "provided class is not a CommonAPI proxy");

    using Traits = safl::detail::FunctionTraits<decltype(func)>;
    using CbType = typename Traits::template Arg<Traits::NrArgs::value - 2>;

    return detail::futurize(proxy, func, detail::CbWrapper<CbType>{}, std::forward<tArgs>(args)...);
}

} // namespace capi
} // namespace safl
