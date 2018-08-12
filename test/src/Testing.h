/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Code to test:
#include <safl/Future.h>
#include <safl/Composition.h>

// Safl includes:
#include <safl/Testing.h>

// Std includes:
#include <map>
#include <functional>

#define EXPECT_FUTURE_FULFILLED() EXPECT_SMTH_INVOKED()
#define EXPECT_NO_FULFILLED_FUTURES() EXPECT_NOTHING_INVOKED()

using namespace safl;

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

inline std::ostream &operator<<(std::ostream &os, const MyInt &value)
{
    return os << "MyInt(" << value.value() << ")";
}

class SaflTest
        : public safl::testing::Test
{
public:
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
    std::map<int, SharedPromise<MyInt>> m_promises;
};
