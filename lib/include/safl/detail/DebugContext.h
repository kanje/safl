/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

#ifdef SAFL_DEVELOPER
#include <iostream>
#define DLOG(__message) do {                                                   \
    std::cout << "[safl] " << this->alias() << ": " << __message << std::endl; \
} while ( !42 )
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
