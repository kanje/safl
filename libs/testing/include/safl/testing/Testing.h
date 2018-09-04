/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#include <safl/Future.h>
#include <safl/Composition.h>

#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <functional>

#define EXPECT_SMTH_INVOKED() EXPECT_TRUE(processSingle())
#define EXPECT_MANY_INVOKED(__n) EXPECT_TRUE(processMultiple(__n))
#define EXPECT_NOTHING_INVOKED() EXPECT_EQ(0, queueSize())

#define EXPECT_FUTURE_FULFILLED() EXPECT_SMTH_INVOKED()
#define EXPECT_NO_FULFILLED_FUTURES() EXPECT_NOTHING_INVOKED()

namespace safl {
namespace testing {

class Executor;

class MyInt final
{
public:
    explicit MyInt(int value)
        : m_value(value)
    {
    }

    MyInt(const MyInt &o)
        : m_value(o.m_value)
    {
    }

    ~MyInt()
    {
        m_value = 0xBAAD;
    }

    bool operator==(int value) const
    {
        return m_value == value;
    }

    int value() const
    {
        return m_value;
    }

private:
    int m_value;
};

inline std::ostream &operator<<(std::ostream &os, const MyInt &value)
{
    return os << "MyInt(" << value.value() << ")";
}

template<typename tValue>
struct Profut
{
    Promise<tValue> p;
    Future<tValue> f;

    Profut(): f(p.future()) {}
};

template<typename tValue>
struct ProfutVector
{
    std::vector<Promise<tValue>> p;
    std::vector<Future<tValue>> f;

    ProfutVector(std::size_t cnt)
    {
        for ( std::size_t i = 0; i < cnt; i++ ) {
            p.emplace_back();
            f.push_back(p.back().future());
        }
    }
};

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
    std::unique_ptr<Executor> m_executor;
};

} // namespace testing
} // namespace safl
