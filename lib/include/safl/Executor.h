/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "detail/Utils.h"

namespace safl
{

/**
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
    virtual void invoke() = 0;
};

template<typename tFunc>
class Invokable
        : public InvokableNtBase
{
public:
    explicit Invokable(tFunc &&f)
        : m_f(std::forward<tFunc>(f))
    {
    }

    void invoke() override
    {
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
        : m_f(new detail::Invokable<tFunc>(std::forward<tFunc>(f)))
    {
    }

    void invoke()
    {
        m_f->invoke();
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

} // namespace safl
