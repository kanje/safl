/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

#include <safl/detail/UniqueInstance.h>

namespace safl {

class Executor;

namespace qt {

class ExecutorScope
        : private detail::UniqueInstance
{
public:
    ExecutorScope() noexcept;
    ~ExecutorScope();

private:
    Executor *m_oldExecutor;
};

} // namespace qt
} // namespace safl
