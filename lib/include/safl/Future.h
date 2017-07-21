/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "detail/FutureDetail.h"

namespace safl
{

/**
 * @brief The Future.
 */
template<typename ValueType>
class Future final
        : public detail::FutureBase<ValueType>
{
    using detail::FutureBase<ValueType>::FutureBase;

public:
    const ValueType &value() const noexcept
    {
        return this->m_ctx->value();
    }
};

template<>
class Future<void>
        : public detail::FutureBase<void>
{
    using FutureBase<void>::FutureBase;
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
