/*
 * This file is a part of Standalone Future Library (safl).
 */

// Code to test:
#include <safl/Future.h>

// Testing:
#include <gtest/gtest.h>

// Std includes:
#include <queue>

#define EXPECT_FUTURE_FULFILLED() EXPECT_TRUE(processSingle())

using namespace safl;

class TestExecutor final
        : public detail::Executor
{
public:
    void invoke(ContextType *ctx, FunctionType f) noexcept
    {
        m_queue.push(std::make_pair(ctx, f));
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
        auto item = m_queue.front();
        m_queue.pop();
        item.second(item.first);
    }

    std::size_t queueSize() const
    {
        return m_queue.size();
    }

private:
    std::queue<std::pair<ContextType*, FunctionType>> m_queue;
};

class SaflTest
        : public ::testing::Test
{
public:
    SaflTest()
    {
        TestExecutor::set(&m_executor);
    }

    bool processSingle()
    {
        return m_executor.processSingle();
    }

private:
    TestExecutor m_executor;
};

TEST_F(SaflTest, canCreatePromisePod)
{
    Promise<int> p;
}

class MySpecialClass final
{
public:
    explicit MySpecialClass(int) {}
};

TEST_F(SaflTest, canCreatePromiseNoDefaultCtor)
{
    Promise<MySpecialClass> p;
}

TEST_F(SaflTest, valueThenLambda)
{
    Promise<int> p;
    auto f = p.future();

    int calledWith = 0;
    bool secondLambdaCalled = false;
    f.then([&](int value)
    {
        calledWith = value;
    }).then([&]()
    {
        secondLambdaCalled = true;
    });

    EXPECT_EQ(0, calledWith);
    EXPECT_FALSE(secondLambdaCalled);

    p.setValue(42);

    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ(42, calledWith);
    EXPECT_FALSE(secondLambdaCalled);

    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ(42, calledWith);
    EXPECT_TRUE(secondLambdaCalled);
}

int divideByTwo(int value)
{
    return value / 2;
}

TEST_F(SaflTest, setValueBeforeThen)
{
    Promise<int> p;
    auto f = p.future();

    p.setValue(1024);

    int calledWith = 0;
    auto newF = f.then(divideByTwo)
                 .then([&](int value)
    {
        calledWith = value;
        return 72;
    });

    // divideByTwo(1024) shall be called
    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ(0, calledWith);

    // our lambda shall be called with the result of 1024/2
    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ(512, calledWith);

    newF.then([&](int value)
    {
        calledWith = value;
    });

    // our second lambda shall be called with the return value
    // of the lambda above
    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ(72, calledWith);
}
