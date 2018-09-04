/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#include <safl/testing/Testing.h>

using namespace safl;
using namespace safl::testing;

namespace {

class CoreTest
        : public Test
{
};

} // anonymous namespace

TEST_F(CoreTest, canCreatePromisePod)
{
    Promise<int> p;
}

TEST_F(CoreTest, canCreatePromiseNoDefaultCtor)
{
    Promise<MyInt> p;
}

TEST_F(CoreTest, fulfilledPromiseWithoutFuture)
{
    Promise<int> p;
    p.setValue(32);
}

TEST_F(CoreTest, valueThenLambda)
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

TEST_F(CoreTest, lvalueLambda)
{
    Promise<int> p;
    auto f = p.future();

    int calledWith = 0;

    {
        /* f.then() must make a copy of this lambda because it will be destroyed
         * when we leave the scope and the multiplier will not be valid any more. */
        auto lambda = [&calledWith, multiplier = std::make_shared<MyInt>(2)](int value)
        {
            calledWith = value * multiplier->value();
        };
        f.then(lambda);
    }

    p.setValue(21);

    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ(42, calledWith);
}

static int divideByTwo(int value)
{
    return value / 2;
}

TEST_F(CoreTest, setValueBeforeThen)
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

TEST_F(CoreTest, futureThenFuture)
{
    Promise<MyInt> p1;
    SharedPromise<std::string> p2;

    auto f = p1.future();

    int calledWith = 0;
    std::string calledString;
    f.then([&calledWith, p2](const MyInt &value) -> Future<std::string>
    {
        calledWith = value.value();
        return p2->future();
    }).then([&](const std::string &value)
    {
        calledString = value;
    });

    EXPECT_NO_FULFILLED_FUTURES();

    p1.setValue(MyInt(1986));
    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ(1986, calledWith);
    EXPECT_TRUE(calledString.empty());

    EXPECT_NO_FULFILLED_FUTURES();

    p2->setValue("hello, world");

    /* Only one future must be fulfilled. Multiple dispatching for queueing
     * futures is not acceptable! */
    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ("hello, world", calledString);
}

TEST_F(CoreTest, basicOnError)
{
    Promise<double> p;
    Future<double> f = p.future();

    int calledWithInt = 0;
    std::string calledWithString;

    f.onError([&](int error)
    {
        calledWithInt = error;
        return 4.2;
    }).onError([&](const std::string &error)
    {
        calledWithString = error;
        return 7.6;
    });

    p.setError(std::string("hello, world"));
    EXPECT_FUTURE_FULFILLED();
    ASSERT_TRUE(f.isReady());
    EXPECT_DOUBLE_EQ(7.6, f.value());
    EXPECT_EQ(0, calledWithInt);
    EXPECT_EQ("hello, world", calledWithString);
}

TEST_F(CoreTest, onErrorWithVoidFuture)
{
    Promise<void> p;
    Future<void> f = p.future();

    int calledWithInt = 0;
    f.onError([&](int error)
    {
        calledWithInt = error;
    });

    p.setError(99);
    EXPECT_FUTURE_FULFILLED();
    ASSERT_TRUE(f.isReady());
    EXPECT_EQ(99, calledWithInt);
}

TEST_F(CoreTest, setErrorBeforeOnError)
{
    Promise<int> p;
    Future<int> f = p.future();

    f.onError([](double)
    {
        std::abort();
        return 99;
    });

    p.setError(MyInt(42));
    EXPECT_NO_FULFILLED_FUTURES();

    f.onError([](int)
    {
        std::abort();
        return 5;
    });
    EXPECT_NO_FULFILLED_FUTURES();

    int calledWithInt = 0;
    f.onError([&](const MyInt &error)
    {
        calledWithInt = error.value();
        return 10;
    });
    EXPECT_FUTURE_FULFILLED();
    ASSERT_TRUE(f.isReady());
    EXPECT_EQ(42, calledWithInt);
    EXPECT_EQ(10, f.value());
}

TEST_F(CoreTest, errorWithMultipleFutures)
{
    Promise<int> p;
    Future<int> f1 = p.future();

    f1.onError([](int)
    {
        std::abort();
        return 5;
    });

    Future<int> f2 = f1.then([](int)
    {
        return 42;
    });

    int calledWithInt = 0;
    f2.onError([&](const MyInt &error)
    {
        calledWithInt = error.value();
        return 99;
    });

    EXPECT_NO_FULFILLED_FUTURES();

    p.setError(MyInt(1022));
    EXPECT_FUTURE_FULFILLED();
    EXPECT_TRUE(f2.isReady());
    EXPECT_EQ(1022, calledWithInt);
    EXPECT_EQ(99, f2.value());
}

TEST_F(CoreTest, errorBeforeThenAndOnError)
{
    Promise<int> p;
    Future<int> f1 = p.future();
    p.setError(-42);
    EXPECT_NO_FULFILLED_FUTURES();

    Future<void> f2 = f1.then([](int){});

    int calledWithInt = 0;
    f2.onError([&](int error)
    {
        calledWithInt = error;
    });
    EXPECT_FUTURE_FULFILLED();
    EXPECT_EQ(-42, calledWithInt);
}

TEST_F(CoreTest, brokenPromise)
{
    bool isPromiseBroken = false;
    SharedPromise<MyInt> p;

    auto f = p->future();
    f.onError([&](BrokePromise)
    {
        isPromiseBroken = true;
        return MyInt(76);
    });

    p.forget();
    EXPECT_FUTURE_FULFILLED();
    EXPECT_TRUE(isPromiseBroken);
}

TEST_F(CoreTest, basicMessage)
{
    Promise<int> p;
    Future<int> f = p.future();

    int calledWithInt = 0;
    p.onMessage([&](const MyInt &i)
    {
        calledWithInt = i.value();
    });
    EXPECT_NO_FULFILLED_FUTURES();

    f.sendMessage(MyInt(42));
    EXPECT_SMTH_INVOKED();
    EXPECT_EQ(42, calledWithInt);
}

TEST_F(CoreTest, messageWithFutureChain)
{
    Promise<int> p;
    Future<int> f1 = p.future();

    int calledWithInt = 0;

    auto f2 = f1.then([&](int i)
    {
        calledWithInt = i;
    });

    p.onMessage([&](const MyInt &i)
    {
        calledWithInt += i.value();
    });
    EXPECT_NO_FULFILLED_FUTURES();

    /* Even though we are sending a message through the last future, it must be
     * processed by the original promise. */
    f2.sendMessage(MyInt(42));
    EXPECT_SMTH_INVOKED();
    EXPECT_EQ(42, calledWithInt);
}

TEST_F(CoreTest, collectEmpty)
{
    std::vector<Future<int>> fs;

    bool isCalled = false;
    auto f = collect(fs).then([&](const std::vector<int> &values)
    {
        if ( values.empty() ) {
            isCalled = true;
        }
    });

    EXPECT_FUTURE_FULFILLED();
    EXPECT_TRUE(isCalled);
}

TEST_F(CoreTest, collectDestroyed)
{
    ProfutVector<MyInt> v(3);
    collect(v.f);
}

TEST_F(CoreTest, collectSuccess)
{
    ProfutVector<MyInt> v(3);

    std::size_t inSize;
    int inValues[3] = {0, 0, 0};
    auto f = collect(v.f).then([&](const std::vector<MyInt> &values)
    {
        inSize = values.size();
        if ( inSize == 3 ) {
            inValues[0] = values[0].value();
            inValues[1] = values[1].value();
            inValues[2] = values[2].value();
        }
    });
    EXPECT_NO_FULFILLED_FUTURES();

    v.p[1].setValue(MyInt(10));
    EXPECT_NO_FULFILLED_FUTURES();

    v.p[0].setValue(MyInt(1));
    EXPECT_NO_FULFILLED_FUTURES();

    v.p[2].setValue(MyInt(100));
    EXPECT_FUTURE_FULFILLED();

    EXPECT_EQ(3, inSize);
    EXPECT_EQ(1, inValues[0]);
    EXPECT_EQ(10, inValues[1]);
    EXPECT_EQ(100, inValues[2]);
}

TEST_F(CoreTest, collectVoid)
{
    ProfutVector<void> v(3);

    bool collectCalled = false;
    auto f = collect(v.f).then([&]()
    {
        collectCalled = true;
    });
    EXPECT_NO_FULFILLED_FUTURES();

    v.p[1].setValue();
    EXPECT_NO_FULFILLED_FUTURES();

    v.p[0].setValue();
    EXPECT_NO_FULFILLED_FUTURES();

    v.p[2].setValue();
    EXPECT_FUTURE_FULFILLED();

    EXPECT_TRUE(collectCalled);
}

TEST_F(CoreTest, collectError)
{
    ProfutVector<int> v(3);

    bool isCalled = false;
    bool isError = false;
    auto f = collect(v.f).then([&](const std::vector<int> &)
    {
        isCalled = true;
    }).onError([&](const std::string &)
    {
        isError = true;
    });

    v.p[0].setValue(10);
    EXPECT_NO_FULFILLED_FUTURES();

    v.p[1].setError(std::string("ERROR"));
    EXPECT_FUTURE_FULFILLED();

    v.p[2].setValue(12);
    EXPECT_NO_FULFILLED_FUTURES();

    EXPECT_FALSE(isCalled);
    EXPECT_TRUE(isError);
}

TEST_F(CoreTest, collectMessage)
{
    ProfutVector<int> v(2);

    int inMsg0 = 0;
    int inMsg1 = 0;
    v.p[0].onMessage([&](int i) { inMsg0 = i; });
    v.p[1].onMessage([&](int i) { inMsg1 = i; });

    auto f = collect(v.f);
    f.sendMessage(42);
    EXPECT_MANY_INVOKED(2);
    EXPECT_EQ(42, inMsg0);
    EXPECT_EQ(42, inMsg1);
}
