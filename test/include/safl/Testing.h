/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Testing:
#include <gtest/gtest.h>

#define EXPECT_SMTH_INVOKED() EXPECT_TRUE(processSingle())
#define EXPECT_MANY_INVOKED(__n) EXPECT_TRUE(processMultiple(__n))
#define EXPECT_NOTHING_INVOKED() EXPECT_EQ(0, queueSize())

namespace safl {
namespace testing {

namespace detail {
class TestExecutor;
} // namespace detail

class Test
        : public ::testing::Test
{
public:
    Test() noexcept;
    ~Test() noexcept;

    bool processSingle() noexcept;
    bool processMultiple(std::size_t cnt) noexcept;
    std::size_t queueSize() noexcept;

private:
    std::unique_ptr<detail::TestExecutor> m_executor;
};

} // namespace testing
} // namespace safl
