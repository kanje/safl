/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "Context.h"
#include "NonCopyable.h"

namespace safl {

/**
 * @internal
 * @brief The implementation detail namespace.
 */
namespace detail {

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
        return *static_cast<Future<tValueType>*>(this);
    }

public:
    FutureBase(ContextType *ctx)
        : m_ctx(ctx)
    {
        m_ctx->attachFuture();
    }

    FutureBase(FutureBase &&other)
        : m_ctx(other.m_ctx)
    {
        other.m_ctx = nullptr;
    }

    ~FutureBase()
    {
        if ( m_ctx ) {
            m_ctx->detachFuture();
        }
    }

    [[ gnu::warn_unused_result ]]
    ContextType *makeShadowOf(ContextType *ctx) noexcept
    {
        ContextType *tmp = m_ctx;
        m_ctx->makeShadowOf(ctx);
        m_ctx = nullptr;
        return tmp;
    }

protected:
    ContextType *m_ctx;
};

/*******************************************************************************
 * Base classes for Promise.
 */

class BrokenPromise {};

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
        m_ctx->attachPromise();
    }

    ~PromiseBase() noexcept
    {
        if ( m_ctx ) {
            if ( m_ctx->isFulfillable() && !m_ctx->isReady() ) {
                setError(BrokenPromise{});
            }
            m_ctx->detachPromise();
        }
    }

protected:
    ContextType *m_ctx;
};

} // namespace detail
} // namespace safl
