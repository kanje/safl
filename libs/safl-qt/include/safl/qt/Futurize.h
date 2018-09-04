/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

#include <safl/Future.h>

#include <QObject>

namespace safl {
namespace qt {

template<typename tObject, typename... tArgs>
auto futurize(tObject *object, void(tObject::*signal)(tArgs...)) noexcept
{
    SharedPromise<std::tuple<tArgs...>> p;

    QObject::connect(object, signal, [p](const tArgs&... args) mutable {
        p->setValue(std::make_tuple(args...));
    });

    return p->future();
}

template<typename tObject, typename tArg>
auto futurize(tObject *object, void(tObject::*signal)(tArg)) noexcept
{
    SharedPromise<tArg> p;

    QObject::connect(object, signal, [p](const tArg &arg) mutable {
        p->setValue(arg);
    });

    return p->future();
}

template<typename tObject>
auto futurize(tObject *object, void(tObject::*signal)()) noexcept
{
    SharedPromise<void> p;

    QObject::connect(object, signal, [p]() mutable {
        p->setValue();
    });

    return p->future();
}

} // namespace qt
} // namespace safl
