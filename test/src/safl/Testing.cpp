/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Self-include:
#include <safl/Testing.h>

// Safl includes:
#include <safl/Executor.h>
#include <safl/detail/DebugContext.h>

// Std includes:
#include <queue>

namespace safl {
namespace testing {
namespace detail {

class TestExecutor final
        : public Executor
{
public:
    void invoke(Task &&task) noexcept override;
    bool processSingle();
    void processNext();
    std::size_t queueSize() const;

private:
    std::queue<Task> m_queue;
};

} // namespace detail
} // namespace testing
} // namespace safl

using namespace safl;
using namespace safl::testing;
using namespace safl::testing::detail;

void TestExecutor::invoke(Task &&task) noexcept
{
    m_queue.push(std::move(task));
}

bool TestExecutor::processSingle()
{
    if ( m_queue.size() != 1 )
    {
        return false;
    }

    processNext();
    return true;
}

void TestExecutor::processNext()
{
    auto f = std::move(m_queue.front());
    m_queue.pop();
    f.invoke();
}

std::size_t TestExecutor::queueSize() const
{
    return m_queue.size();
}

Test::Test() noexcept
    : m_executor(std::make_unique<TestExecutor>())
{
    TestExecutor::setInstance(m_executor.get());
    safl::detail::DebugContext::resetCounters();
}

Test::~Test() noexcept
{
    EXPECT_NOTHING_INVOKED();

#ifdef SAFL_DEVELOPER
    /* Simple test for memory leaks. */
    assert(safl::detail::DebugContext::cntContexts() == 0);
#endif
}

bool Test::processSingle() noexcept
{
    return m_executor->processSingle();
}

std::size_t Test::queueSize() noexcept
{
    return m_executor->queueSize();
}
