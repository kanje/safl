/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "Context.h"
#include "NonCopyable.h"

namespace safl {

template<typename tValueType>
class Promise;

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

    template<typename tFunc>
    auto &onMessage(tFunc &&f) noexcept
    {
        m_ctx->onMessage(std::forward<tFunc>(f));
        return *static_cast<Promise<tValueType>*>(this);
    }

protected:
    PromiseBase() noexcept
        : m_ctx(new InitialContext<tValueType>())
    {
        m_ctx->attachPromise();
    }

    PromiseBase(PromiseBase &&other) noexcept
        : m_ctx(other.m_ctx)
    {
        other.m_ctx = nullptr;
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
