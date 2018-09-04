/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#include "Testing.h"

#include "MyObject.h"

#include <safl/qt/Executor.h>
#include <safl/qt/Futurize.h>
#include <safl/Composition.h>

#include <QCoreApplication>
#include <QTimer>

namespace {

int s_argc = 1;
char s_arg1[] = "ut-safl-qt";
char *s_argv[] = {s_arg1, nullptr};

class QtTest
        : public ::testing::Test
{
public:
    QtTest() noexcept
        : m_app(s_argc, s_argv)
    {
    }

    void later(std::function<void()> f)
    {
        QTimer::singleShot(0, [f]() { f(); });
    }

    int inloop(std::function<void()> f)
    {
        later(f);
        return m_app.exec();
    }

private:
    QCoreApplication m_app;
    safl::qt::ExecutorScope m_qtExecutor;
};

} // anonymous namespace

TEST_F(QtTest, executor)
{
    Profut<int> pf;

    pf.f.then([](int i)
    {
        qApp->exit(i);
    });

    int rv = inloop([&]()
    {
        pf.p.setValue(99);
    });

    EXPECT_EQ(99, rv);
}

TEST_F(QtTest, signal0)
{
    MyObject object;

    safl::qt::futurize(&object, &MyObject::sig0).then([]()
    {
        qApp->exit(86);
    });

    int rv = inloop([&]()
    {
        object.sig0();
    });

    EXPECT_EQ(86, rv);
}

TEST_F(QtTest, signal1)
{
    MyObject object;

    safl::qt::futurize(&object, &MyObject::sig1).then([](int i)
    {
        qApp->exit(2 * i);
    });

    int rv = inloop([&]()
    {
        object.sig1(40);
    });

    EXPECT_EQ(80, rv);
}

TEST_F(QtTest, signal2)
{
    MyObject object;

    safl::qt::futurize(&object, &MyObject::sig2)
            .then([&](const std::tuple<int, int> &t)
    {
        qApp->exit(std::get<0>(t) + std::get<1>(t));
    });

    int rv = inloop([&]()
    {
        object.sig2(40, 3);
    });

    EXPECT_EQ(43, rv);
}

TEST_F(QtTest, waitTwoSignals)
{
    MyObject object;

    std::vector<safl::Future<int>> futures;

    futures.push_back(safl::qt::futurize(&object, &MyObject::sig0)
                      .then([]() { return 76; }));
    futures.push_back(safl::qt::futurize(&object, &MyObject::sig1));

    safl::collect(futures).then([&](const std::vector<int> &results)
    {
        qApp->exit(results[0] + results[1]);
    });

    int rv = inloop([&]()
    {
        object.sig1(100);
        later([&]()
        {
            object.sig0();
        });
    });

    EXPECT_EQ(176, rv);
}

TEST_F(QtTest, waitTwoVoidSignals)
{
    MyObject object;

    std::vector<safl::Future<void>> futures;

    futures.push_back(safl::qt::futurize(&object, &MyObject::sig0));
    futures.push_back(safl::qt::futurize(&object, &MyObject::sig1)
                      .then([](int){}));

    safl::collect(futures).then([&]()
    {
        qApp->exit(77);
    });

    int rv = inloop([&]()
    {
        object.sig1(100);
        later([&]() {
            object.sig0();
        });
    });

    EXPECT_EQ(77, rv);
}
