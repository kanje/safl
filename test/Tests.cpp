/*
 * This file is a part of Standalone Future Library (safl).
 */

// Code to test:
#include <safl/Future.h>

// Testing:
#include <gtest/gtest.h>

// Std includes:
#include <queue>
#include <map>

#define EXPECT_FUTURE_FULFILLED() EXPECT_TRUE(processSingle())
#define EXPECT_NO_FULFILLED_FUTURES() EXPECT_EQ(0, queueSize())

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

class MyInt final
{
public:
    explicit MyInt(int value)
        : m_value(value)
    {
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

std::ostream &operator<<(std::ostream &os, const MyInt &value)
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

TEST_F(SaflTest, canCreatePromisePod)
{
    Promise<int> p;
}

TEST_F(SaflTest, canCreatePromiseNoDefaultCtor)
{
    Promise<MyInt> p;
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

TEST_F(SaflTest, futureThenFuture)
{
    SharedPromise<std::string> p;

    auto f = future(0);

    int calledWith = 0;
    std::string calledString;
    f.then([&calledWith, p](const MyInt &value) -> Future<std::string>
    {
        calledWith = value.value();
        return p->future();
    }).then([&](const std::string &value)
    {
        calledString = value;
    });

    EXPECT_NO_FULFILLED_FUTURES();

    setValue(0, 1986);
    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ(1986, calledWith);
    EXPECT_TRUE(calledString.empty());

    EXPECT_NO_FULFILLED_FUTURES();

    p->setValue("hello, world");

    /* Only one future must be fulfilled. Multiple dispatching for queueing
     * futures is not acceptable! */
    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ("hello, world", calledString);
}
