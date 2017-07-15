/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "Utils.h"

namespace safl
{
namespace detail
{

/**
 * @internal
 * @defgroup Exec Executors
 * @{
 */

class Invokable;

namespace detail
{

class InvokableNtBase
        : private UniqueInstance
{
public:
    virtual ~InvokableNtBase() = default;
    virtual void invokex() = 0;
};

template<typename tFunc>
class xInvokable
        : public InvokableNtBase
{
public:
    explicit xInvokable(tFunc &&f)
        : m_f(std::forward<tFunc>(f))
    {
        static_assert(!std::is_same<tFunc, Invokable&>::value, "bbb");
    }

    void invokex() override
    {
        static_assert(!std::is_same<tFunc, Invokable&>::value, "aaa");
        m_f();
    }

private:
    std::decay_t<tFunc> m_f;
};

} // namespace detail

class Invokable
{
public:
    template<typename tFunc>
    explicit Invokable(tFunc &&f)
        : m_f(new detail::xInvokable<tFunc>(std::forward<tFunc>(f)))
    {
        static_assert(!std::is_same<tFunc, Invokable&>::value, "ccc");
    }

    void invoke()
    {
        m_f->invokex();
    }

private:
    std::unique_ptr<detail::InvokableNtBase> m_f;
};

class Executor
{
public:
    static void set(Executor *executor) noexcept;
    virtual void invoke(Invokable &&f) noexcept = 0;

    template<typename tFunc>
    void invoke(tFunc &&f)
    {
        invoke(Invokable(std::forward<tFunc>(f)));
    }

protected:
    ~Executor() noexcept = default;
};

/// @}

} // namespace detail
} // namespace safl
