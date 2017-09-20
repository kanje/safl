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

namespace safl
{
namespace detail
{

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
    static std::size_t cntContexts() noexcept;
    static void resetCounters() noexcept;

protected:
    DebugContext() noexcept;
    ~DebugContext() noexcept;

private:
    unsigned int m_alias;
};

/**
 * @internal
 * @brief The base class for non-copyable but movable classes.
 *
 * The classes which must be not copyable but movable, must @e privately inherit
 * from this class.
 */

/**
 * @internal
 * @brief The base class for non-copyable and non-movable classes.
 *
 * The classes which must be neither copyable nor movable, must @e privately
 * inherit from this class.
 */
/**
 * @internal
 * @brief The base for classes which operate on or store an erased type.
 */

/// @}

} // namespace detail
} // namespace safl
