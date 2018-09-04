/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "detail/UniqueInstance.h"

// Std includes:
#include <memory>

namespace safl {

/**
 * @defgroup Exec Executors
 * @{
 */

namespace detail
{

class InvocableNtBase
        : private UniqueInstance
{
public:
    virtual ~InvocableNtBase() = default;
    virtual void invoke() = 0;
};

template<typename tFunc>
class Invocable
        : public InvocableNtBase
{
public:
    explicit Invocable(tFunc &&f)
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

class Task
{
public:
    template<typename tFunc>
    Task(tFunc &&f)
        : m_f(std::make_unique<Invocable<tFunc>>(std::forward<tFunc>(f)))
    {
    }

    void invoke()
    {
        m_f->invoke();
    }

private:
    std::unique_ptr<InvocableNtBase> m_f;
};

} // namespace detail

class Executor
        : private detail::UniqueInstance
{
public:
    using Task = detail::Task;

public:
    virtual void invoke(Task &&task) noexcept = 0;

public:
    static void setInstance(Executor *executor) noexcept;
    static Executor *instance() noexcept;

protected:
    ~Executor() noexcept = default;
};

/// @}

} // namespace safl
