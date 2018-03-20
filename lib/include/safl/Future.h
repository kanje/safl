/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "detail/FutureDetail.h"

namespace safl {

/**
 * @brief The Future.
 */
template<typename tValueType>
class Future
        : public detail::FutureNtBase
{
public:
    using ValueType = tValueType;
    using ContextType = detail::ContextBase<ValueType>;

public:
    /**
     * @brief Specify a continuation.
     */
    template<typename tFunc>
    auto then(tFunc &&f) noexcept
    {
        return m_ctx->then(std::forward<tFunc>(f));
    }

    /**
     * @brief Specify an error handler.
     */
    template<typename tFunc>
    auto &onError(tFunc &&f) noexcept
    {
        m_ctx->onError(std::forward<tFunc>(f));
        return *static_cast<Future<tValueType>*>(this);
    }

    /**
     * @brief Send a message to an asynchronous operation.
     */
    template<typename tMessage>
    void sendMessage(tMessage &&msg) noexcept
    {
        m_ctx->sendMessage(std::forward<tMessage>(msg));
    }

    /**
     * @brief Check if this @future is ready.
     */
    bool isReady() const noexcept
    {
        return m_ctx->isReady();
    }

    /**
     * @brief Get the value.
     */
    template<typename xValueType = tValueType,
             typename = std::enable_if_t<!std::is_void<xValueType>::value>>
    const xValueType &value() const noexcept
    {
        return this->m_ctx->value();
    }

public:
    Future(ContextType *ctx)
        : m_ctx(ctx)
    {
        m_ctx->attachFuture();
    }

    Future(Future &&other)
        : m_ctx(other.m_ctx)
    {
        other.m_ctx = nullptr;
    }

    ~Future()
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

using BrokePromise = detail::BrokenPromise;

/**
 * @brief The Promise.
 */
template<typename tValueType>
class Promise final
        : public detail::PromiseBase<tValueType>
{
public:
    void setValue(const tValueType &value) noexcept
    {
        this->m_ctx->setValue(value);
    }
};

template<>
class Promise<void>
        : public detail::PromiseBase<void>
{
public:
    void setValue() noexcept
    {
        this->m_ctx->setValue();
    }
};



template<typename tValueType>
class SharedPromise final
{
public:
    SharedPromise()
        : m_p(std::make_shared<Promise<tValueType>>())
    {
    }

    Promise<tValueType> *operator->() noexcept
    {
        return m_p.get();
    }

    const Promise<tValueType> *operator->() const noexcept
    {
        return m_p.get();
    }

    void forget() noexcept
    {
        m_p.reset();
    }

private:
    std::shared_ptr<Promise<tValueType>> m_p;
};

} // namespace safl
