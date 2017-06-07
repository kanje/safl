/*
 * This file is a part of Standalone Future Library (safl).
 */

#pragma once

// Local includes:
#include "FunctionTraits.h"

// Std includes:
#include <iostream>

char mnemo(void *);
#define DLOG(__message) do { std::cout << "[safl] " << mnemo(static_cast<ContextNtBase*>(this)) << ": " << __message << std::endl; } while ( !42 )

namespace safl
{

template<typename ValueType>
class Future;

namespace detail
{

/*******************************************************************************
 * Non-templated base classes for futures, promises and contexts.
 */

class FutureNtBase
{
public:
    // non-copyable
    FutureNtBase(const FutureNtBase&) = delete;
    FutureNtBase &operator=(const FutureNtBase&) = delete;

    // movable
    FutureNtBase(FutureNtBase&&) noexcept;
    FutureNtBase &operator=(FutureNtBase&&) noexcept;

protected:
    FutureNtBase() noexcept = default;
    ~FutureNtBase() noexcept = default;
};


class PromiseNtBase
{
public:
    PromiseNtBase(const PromiseNtBase&) = delete;
    PromiseNtBase(PromiseNtBase&&) = default;
    PromiseNtBase &operator=(const PromiseNtBase&) = delete;

protected:
    PromiseNtBase() noexcept = default;
    ~PromiseNtBase() noexcept = default;
};

class ContextNtBase
{
public:
    ContextNtBase(const ContextNtBase&) = delete;
    ContextNtBase &operator=(const ContextNtBase&) = delete;

public:
    bool isReady() const noexcept
    {
        return m_isValueSet;
    }

    void setValue() noexcept;
    void makeShadowOf(ContextNtBase *next) noexcept;

protected:
    ContextNtBase() noexcept;
    virtual ~ContextNtBase() noexcept;
    void setTarget(ContextNtBase *next) noexcept;

private:
    void fulfil() noexcept;
    virtual void acceptInput() noexcept = 0;

protected:
    ContextNtBase *m_prev;
    ContextNtBase *m_next;
    bool m_isValueSet;
    bool m_isShadowed;
};

/*******************************************************************************
 * Base classes for contexts.
 */

template<typename ValueType>
class ContextValueBase
        : public ContextNtBase
{
public:
    const ValueType &value() const noexcept
    {
        return *reinterpret_cast<const ValueType*>(m_value);
    }

    void setValue(const ValueType &value) noexcept
    {
        DLOG(">> setValue: " << value);
        new (&m_value) ValueType(value);
        ContextNtBase::setValue();
        DLOG("<< setValue");
    }

private:
    char m_value[sizeof(ValueType)];
};

template<>
class ContextValueBase<void>
        : public ContextNtBase
{
};

template<typename ValueType, typename Func, typename InputType>
class SyncNextContext;

template<typename ValueType, typename Func, typename InputType>
class AsyncNextContext;

template<typename tFunc>
struct ThenHelper final
{
    using ResultType = typename FunctionTraits<tFunc>::ReturnType;
    using IsResultFuture = std::is_base_of<FutureNtBase, ResultType>;
    using FutureType = std::conditional_t<IsResultFuture::value,
                                          ResultType, Future<ResultType>>;
    using ValueType = typename FutureType::ValueType;

    template<typename xValueType, typename xFunc, typename xInputType>
    using NextContextType = std::conditional_t<IsResultFuture::value,
                                               AsyncNextContext<xValueType, xFunc, xInputType>,
                                               SyncNextContext<xValueType, xFunc, xInputType>>;
};

template<typename ValueType>
class ContextBase
        : public ContextValueBase<ValueType>
{
public:
    template<typename Func, typename Then = ThenHelper<Func>>
    typename Then::FutureType then(Func &&f)
    {
        DLOG(">> then");
        auto next = new typename Then::template NextContextType
                <typename Then::ValueType, Func, ValueType>(std::forward<Func>(f));
        this->setTarget(next);
        DLOG("<< then");
        return { next };
    }
};

/*******************************************************************************
 * Concrete future contexts.
 */

template<typename ValueType>
class InitialContext final
        : public ContextBase<ValueType>
{
private:
    void acceptInput() noexcept override
    {
        // do nothing
    }
};

template<typename ValueType, typename Func, typename InputType>
class NextContextBase
        : public ContextBase<ValueType>
{
public:
    NextContextBase(Func &&f)
        : m_f(std::forward<Func>(f))
    {
    }

protected:
    auto prev() const noexcept
    {
        return static_cast<ContextValueBase<InputType>*>(this->m_prev);
    }

protected:
    Func m_f;
};

template<typename ValueType, typename Func, typename InputType>
class SyncNextContext final
        : public NextContextBase<ValueType, Func, InputType>
{
    using NextContextBase<ValueType, Func, InputType>::NextContextBase;

private:
    void acceptInput() noexcept override
    {
        DLOG(">> acceptInput");
        this->setValue(this->m_f(this->prev()->value()));
        DLOG("<< acceptInput");
    }
};

template<typename Func, typename InputType>
class SyncNextContext<void, Func, InputType>
        : public NextContextBase<void, Func, InputType>
{
    using NextContextBase<void, Func, InputType>::NextContextBase;

private:
    void acceptInput() noexcept override
    {
        DLOG(">> acceptInput (void output)");
        this->m_f(this->prev()->value());
        this->setValue();
        DLOG("<< acceptInput (void output)");
    }
};

template<typename ValueType, typename Func>
class SyncNextContext<ValueType, Func, void>
        : public NextContextBase<ValueType, Func, void>
{
    using NextContextBase<ValueType, Func, void>::NextContextBase;

private:
    void acceptInput() noexcept override
    {
        DLOG(">> acceptInput (void input)");
        this->setValue(this->m_f());
        DLOG("<< acceptInput (void input)");
    }
};

template<typename Func>
class SyncNextContext<void, Func, void>
        : public NextContextBase<void, Func, void>
{
    using NextContextBase<void, Func, void>::NextContextBase;

private:
    void acceptInput() noexcept override
    {
        DLOG(">> acceptInput (void input and output)");
        this->m_f();
        this->setValue();
        DLOG("<< acceptInput (void input and output)");
    }
};

template<typename ValueType, typename Func, typename InputType>
class AsyncNextContext final
        : public NextContextBase<ValueType, Func, InputType>
{
    using NextContextBase<ValueType, Func, InputType>::NextContextBase;

private:
    void acceptInput() noexcept override
    {
        if ( m_shadow == nullptr )
        {
            DLOG(">> acceptInput (create shadow)");
            m_shadow = this->m_f(this->prev()->value()).makeShadowOf(this);
            DLOG("<< acceptInput (create shadow)");
        }
        else
        {
            DLOG(">> acceptInput (process shadow)");
            this->setValue(m_shadow->value());
            DLOG("<< acceptInput (process shadow)");
        }
    }

private:
    ContextBase<ValueType> *m_shadow = nullptr;
};

/*******************************************************************************
 * Base classes for futures and promises.
 */

template<typename InValueType>
class FutureBase
        : public FutureNtBase
{
public:
    using ValueType = InValueType;
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

public:
    FutureBase(ContextType *ctx)
        : m_ctx(ctx)
    {
    }

    ContextType *makeShadowOf(ContextType *ctx)
    {
        ContextType *tmp = m_ctx;
        m_ctx->makeShadowOf(ctx);
        m_ctx = nullptr;
        return tmp;
    }

protected:
    ContextType *m_ctx;
};

template<typename ValueType>
class PromiseBase
        : public PromiseNtBase
{
public:
    using Context = ContextBase<ValueType>;

public:
    PromiseBase() noexcept
        : m_ctx(new InitialContext<ValueType>())
    {
    }

    Future<ValueType> future() const noexcept
    {
        return { m_ctx };
    }

protected:
    Context *m_ctx;
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
