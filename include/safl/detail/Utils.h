/*
 * This file is a part of Standalone Future Library (safl).
 */

#pragma once

// Std includes:
#include <typeindex>

namespace safl
{
namespace detail
{

/**
 * @defgroup Util Utility Classes.
 * @{
 */

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
    NonCopyable(NonCopyable&&) noexcept = default;
    NonCopyable &operator=(NonCopyable&&) noexcept = default;

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
