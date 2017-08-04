/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Code to test:
#include <safl/Future.h>
#include <safl/Executor.h>

// Testing:
#include <gtest/gtest.h>

// Std includes:
#include <queue>
#include <map>
#include <functional>

#define EXPECT_FUTURE_FULFILLED() EXPECT_TRUE(processSingle())
#define EXPECT_NO_FULFILLED_FUTURES() EXPECT_EQ(0, queueSize())

using namespace safl;

class TestExecutor final
        : public Executor
{
public:
    void invoke(Invokable &&f) noexcept
    {
        m_queue.push(std::move(f));
    }

    bool processSingle()
    {
        if ( m_queue.size() != 1 )
        {
            return false;
        }

        processNext();
        return true;
    }

    void processNext()
    {
        auto f = std::move(m_queue.front());
        m_queue.pop();
        f.invoke();
    }

    std::size_t queueSize() const
    {
        return m_queue.size();
    }

private:
    std::queue<Invokable> m_queue;
};

class MyInt final
{
public:
    explicit MyInt(int value)
        : m_value(value)
    {
    }

    ~MyInt()
    {
        m_value = 0xbaadf00d;
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

class SaflTest
        : public ::testing::Test
{
public:
    SaflTest()
    {
        TestExecutor::set(&m_executor);
    }

    void TearDown() override
    {
        EXPECT_NO_FULFILLED_FUTURES();

#ifdef SAFL_DEVELOPER
        /* Simple test for memory leaks. */
        ASSERT_EQ(0, detail::ContextNtBase::cntContexts());
#endif
    }

    bool processSingle()
    {
        return m_executor.processSingle();
    }

    std::size_t queueSize()
    {
        return m_executor.queueSize();
    }

    Future<MyInt> future(int key)
    {
        SharedPromise<MyInt> p;
        m_promises[key] = p;
        return p->future();
    }

    void setValue(int key, int value)
    {
        SharedPromise<MyInt> p = m_promises[key];
        m_promises.erase(key);
        p->setValue(MyInt(value));
    }

private:
    TestExecutor m_executor;
    std::map<int, SharedPromise<MyInt>> m_promises;
};
