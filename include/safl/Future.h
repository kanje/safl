/*
 * This file is a part of Standalone Future Library (safl).
 */

#pragma once

// Local includes:
#include "detail/FutureDetail.h"

// Std includes:
#include <memory>

namespace safl
{

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

template<typename ValueType>
class Promise final
        : public detail::PromiseBase<ValueType>
{
public:
    void setValue(const ValueType &value) noexcept
    {
        this->m_ctx->setValue(value);
        this->m_ctx = nullptr;
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
        this->m_ctx = nullptr;
    }
};

template<typename ValueType>
class SharedPromise final
{
public:
    SharedPromise()
        : m_p(std::make_shared<Promise<ValueType>>())
    {
    }
    SharedPromise(const SharedPromise &) = default;
    SharedPromise(SharedPromise &&) = default;

    Promise<ValueType> *operator->() const noexcept
    {
        return m_p.get();
    }

private:
    std::shared_ptr<Promise<ValueType>> m_p;
};

} // namespace safl
