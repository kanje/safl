/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

#ifdef SAFL_DEVELOPER
#include <iostream>
#include <tuple>
#include <vector>

#define DLOG(__message) do {                                                   \
    std::cout << "[safl] " << this->alias() << ": " << __message << std::endl; \
} while ( !42 )

/* This is a helper for printing setValue for vectors. */
template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &values)
{
    os << "vector(";
    bool isFirst = true;
    for ( const auto &value : values ) {
        os << (isFirst ? "" : ", ") << value;
        isFirst = false;
    }
    return os << ")";
}

/* Print tuples. */
template<unsigned N, typename T, bool Last = (N+1 == std::tuple_size<T>::value)>
struct TuplePrinter
{
    static void print(std::ostream &os, const T &t)
    {
        os << std::get<N>(t) << ", ";
        TuplePrinter<N+1, T>::print(os, t);
    }
};
template<unsigned N, typename T>
struct TuplePrinter<N, T, true>
{
    static void print(std::ostream &os, const T &t)
    {
        os << std::get<N>(t);
    }
};
template<typename... Ts>
std::ostream &operator<<(std::ostream &os, const std::tuple<Ts...> &t)
{
    os << "tuple(";
    TuplePrinter<0, std::tuple<Ts...>>::print(os, t);
    return os << ")";
}

#else
#define DLOG(__message) do { /* no-op */ } while ( !42 )
#endif

namespace safl {
namespace detail {

/**
 * @internal
 * @defgroup Util Utility Classes
 * @{
 */

class DebugContext
{
public:
    unsigned int alias() const noexcept
    {
        return m_alias;
    }

public:
    static unsigned int cntContexts() noexcept;
    static void resetCounters() noexcept;

protected:
    DebugContext() noexcept;
    ~DebugContext() noexcept;

private:
    unsigned int m_alias;
};

/// @}

} // namespace detail
} // namespace safl
