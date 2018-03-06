/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Self-include:
#include <safl/Executor.h>

using namespace safl;

static Executor *s_executor;

void Executor::setInstance(Executor *executor) noexcept
{
    s_executor = executor;
}

Executor *Executor::instance() noexcept
{
    return s_executor;
}
