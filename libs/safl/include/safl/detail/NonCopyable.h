/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

namespace safl {
namespace detail {

/**
 * @ingroup Util
 * @brief The base class for non-copyable but movable classes.
 *
 * Classes which must be not copyable but movable must @e privately inherit from
 * this class.
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

} // namespace detail
} // namespace safl
