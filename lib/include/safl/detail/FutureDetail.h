/*
 * This file is a part of Standalone Future Library (safl).
 */

#pragma once

// Local includes:
#include "Context.h"

namespace safl
{

/**
 * @internal
 * @brief The implementation detail namespace.
 */
namespace detail
{

/*******************************************************************************
 * Base classes for Future.
 */

class FutureNtBase
        : private NonCopyable
{
protected:
    FutureNtBase() = default;
    ~FutureNtBase() = default;
};

template<typename tValueType>
class FutureBase
        : public FutureNtBase
{
public:
    using ValueType = tValueType;
    using ContextType = ContextBase<ValueType>;

public:
    bool isReady() const noexcept
    {
        return m_ctx->isReady();
    }

    template<typename Func>
    auto then(Func &&f) noexcept
    {
        return m_ctx->then(std::forward<Func>(f));
    }

    template<typename tFunc>
    auto &onError(tFunc &&f) noexcept
    {
        m_ctx->onError(std::forward<tFunc>(f));
        return *this;
    }

public:
    FutureBase(ContextType *ctx)
        : m_ctx(ctx)
    {
    }

    FutureBase(FutureBase &&other)
        : m_ctx(other.m_ctx)
    {
        other.m_ctx = nullptr;
    }

    [[ gnu::warn_unused_result ]]
    ContextType *makeShadowOf(ContextType *ctx) noexcept
    {
        ContextType *tmp = m_ctx;
        m_ctx->makeShadowOf(ctx);
        m_ctx = nullptr;
        return tmp;
    }

    void detach() noexcept;

protected:
    ContextType *m_ctx;
};

/*******************************************************************************
 * Base classes for Promise.
 */

class PromiseNtBase
        : private NonCopyable
{
protected:
    PromiseNtBase() = default;
    ~PromiseNtBase() = default;
};

template<typename tValueType>
class PromiseBase
        : public PromiseNtBase
{
public:
    using ValueType = tValueType;
    using ContextType = ContextBase<tValueType>;

public:
    Future<ValueType> future() const noexcept
    {
        return { m_ctx };
    }

    template<typename tErrorType>
    void setError(tErrorType &&error) noexcept
    {
        m_ctx->setError(std::forward<tErrorType>(error));
    }

protected:
    PromiseBase() noexcept
        : m_ctx(new InitialContext<tValueType>())
    {
    }

    ~PromiseBase() noexcept
    {
        if ( m_ctx )
        {
            //setError(-1);
        }
    }

protected:
    ContextType *m_ctx;
};

/*******************************************************************************
 * Executor.
 */

class Executor
{
public:
    using ContextType = ContextNtBase;
    using FunctionType = void(*)(ContextNtBase*);

public:
    static void set(Executor *executor) noexcept;
    virtual void invoke(ContextType *ctx, FunctionType f) noexcept = 0;

protected:
    ~Executor() noexcept = default;
};

} // namespace detail
} // namespace safl
