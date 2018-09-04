/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

namespace safl {
namespace detail {

/**
 * @ingroup Util
 * @brief The base class for non-copyable and non-movable classes.
 *
 * Classes which must be neither copyable nor movable must @e privately inherit
 * from this class.
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

} // namespace detail
} // namespace bsl
