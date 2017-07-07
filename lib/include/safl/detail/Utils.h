/*
 * This file is a part of Standalone Future Library (safl).
 */

#pragma once

// Std includes:
#include <typeindex>
#include <iostream>

namespace safl
{
namespace detail
{

/**
 * @internal
 * @defgroup Util Utility Classes
 * @{
 */

char mnemo(void *);
#define DLOG(__message) do { std::cout << "[safl] " << mnemo(static_cast<ContextNtBase*>(this)) << ": " << __message << std::endl; } while ( !42 )

/**
 * @internal
 * @brief The base class for non-copyable but movable classes.
 *
 * The classes which must be not copyable but movable, must @e privately inherit
 * from this class.
 */
class NonCopyable
{
public:
    // non-copyable
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable &operator=(const NonCopyable&) = delete;

    // movable
    NonCopyable(NonCopyable&&) = default;
    NonCopyable &operator=(NonCopyable&&) = default;

protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
};

/**
 * @internal
 * @brief The base class for non-copyable and non-movable classes.
 *
 * The classes which must be neither copyable nor movable, must @e privately
 * inherit from this class.
 */
class UniqueInstance
{
public:
    // non-copyable, non-movable
    UniqueInstance(const UniqueInstance&) = delete;
    UniqueInstance &operator=(const UniqueInstance&) = delete;

protected:
    UniqueInstance() = default;
    ~UniqueInstance() = default;
};

class TypeEraser
{
public:
    template<typename tType>
    bool isType() const
    {
        return m_typeIndex == typeid(tType);
    }

    bool isType(const TypeEraser *other) const
    {
        return m_typeIndex == other->m_typeIndex;
    }

protected:
    TypeEraser(std::type_index typeIndex)
        : m_typeIndex(typeIndex)
    {
    }

    ~TypeEraser() = default;

private:
    std::type_index m_typeIndex;
};

/// @}

} // namespace detail
} // namespace safl
