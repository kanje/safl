/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Std includes:
#include <functional>

namespace safl
{
namespace detail
{

/**
 * @internal
 * @defgroup Exec Executors
 * @{
 */

class Executor
{
public:
    using FunctionType = std::function<void()>;

public:
    static void set(Executor *executor) noexcept;
    virtual void invoke(FunctionType f) noexcept = 0;

protected:
    ~Executor() noexcept = default;
};

/// @}

} // namespace detail
} // namespace safl
