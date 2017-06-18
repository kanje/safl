/*
 * This file is a part of Standalone Future Library (safl).
 */

#pragma once

// Local includes:
#include "FunctionTraits.h"
#include "Utils.h"

// Std includes:
#include <iostream>
#include <vector>
#include <memory>

char mnemo(void *);
#define DLOG(__message) do { std::cout << "[safl] " << mnemo(static_cast<ContextNtBase*>(this)) << ": " << __message << std::endl; } while ( !42 )

namespace safl
{

template<typename ValueType>
class Future;

/**
 * @internal
 * @brief The implementation detail namespace.
 */
namespace detail
{

/*******************************************************************************
 * Non-templated base classes for futures, promises and contexts.
 */

class FutureNtBase
        : private NonCopyable
{
protected:
    FutureNtBase() = default;
    ~FutureNtBase() = default;
};

class PromiseNtBase
        : private NonCopyable
{
protected:
    PromiseNtBase() noexcept = default;
    ~PromiseNtBase() noexcept = default;
};

class StoredErrorNtBase
        : public TypeEraser
{
public:
    StoredErrorNtBase(std::type_index typeIndex, const void *error)
        : TypeEraser(typeIndex)
        , m_error(error)
    {
    }

    const void *rawPointer() const
    {
        return m_error;
    }

private:
    const void *m_error;
};

class ContextNtBase;

class ErrorHandlerNtBase
        : public TypeEraser
{
    using TypeEraser::TypeEraser;

public:
    virtual ~ErrorHandlerNtBase() noexcept;
    virtual void acceptError(ContextNtBase *ctx, const void *error) = 0;

    void acceptError(ContextNtBase *ctx, const StoredErrorNtBase *error)
    {
        acceptError(ctx, error->rawPointer());
    }
};

class ContextNtBase
        : private UniqueInstance
{
public:
    bool isReady() const
    {
        return m_isValueSet;
    }

    void setValue();
    void makeShadowOf(ContextNtBase *next);

protected:
    ContextNtBase();
    virtual ~ContextNtBase();
    void setTarget(ContextNtBase *next);
    void addErrorHandler(std::unique_ptr<ErrorHandlerNtBase> &&errorHandler);
    void storeError(std::unique_ptr<StoredErrorNtBase> &&error);

private:
    void fulfil();
    virtual void acceptInput() = 0;

protected:
    ContextNtBase *m_prev;
    ContextNtBase *m_next;
    bool m_isValueSet;
    bool m_isShadowed;

private: // error handling
    std::unique_ptr<StoredErrorNtBase> m_storedError;
    std::vector<std::unique_ptr<ErrorHandlerNtBase>> m_errorHandlers;
};

/*******************************************************************************
 * Base classes for errors and requests.
 */

template<typename tErrorType>
class StoredError final
        : public StoredErrorNtBase
{
public:
    StoredError(tErrorType &&error)
        : StoredErrorNtBase(typeid(tErrorType),
                            reinterpret_cast<const void*>(&m_error))
        , m_error(std::forward<tErrorType>(error))
    {
    }

private:
    std::remove_reference_t<tErrorType> m_error;
};

template<typename tValueType>
class ContextBase;

template<typename tValueType, typename tFunc>
class ErrorHandler final
        : public ErrorHandlerNtBase
{
    using Traits = FunctionTraits<tFunc>;
    static_assert(Traits::NrArgs::value == 1, "error handler must have exactly one argument");
    static_assert(std::is_same<typename Traits::ReturnType, tValueType>::value,
                  "error handler must return the same type as the corresponding future");

public:
    using ErrorType = typename Traits::FirstArg;
    using ContextType = ContextBase<tValueType>;

public:
    ErrorHandler(tFunc&& f)
        : ErrorHandlerNtBase(typeid(ErrorType))
        , m_f(std::forward<tFunc>(f))
    {
    }

    void acceptError(ContextNtBase *ctx, const void *error) override
    {
        acceptError(static_cast<ContextType*>(ctx),
                    *reinterpret_cast<const ErrorType*>(error));
    }

private:
    template<typename xValueType = tValueType>
    void acceptError(typename std::enable_if_t<std::is_same<xValueType, void>::value,
                                               ContextType> *ctx,
                     const ErrorType &error)
    {
        m_f(error);
        ctx->setValue();
    }

    template<typename xValueType = tValueType>
    void acceptError(typename std::enable_if_t<!std::is_same<xValueType, void>::value,
                                               ContextType> *ctx,
                     const ErrorType &error)
    {
        ctx->setValue(m_f(error));
    }

private:
    std::remove_reference_t<tFunc> m_f;
};

/*******************************************************************************
 * Base classes for contexts.
 */

template<typename ValueType>
class ContextValueBase
        : public ContextNtBase
{
public:
    ~ContextValueBase()
    {
#if 0
        if ( m_isValueSet )
        {
            // TODO: call m_value's destructor
        }
#endif
    }

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

template<typename tValueType>
class ContextBase
        : public ContextValueBase<tValueType>
{
public:
    template<typename tFunc, typename tThen = ThenHelper<tFunc>>
    typename tThen::FutureType then(tFunc &&f)
    {
        DLOG(">> then");
        auto next = new typename tThen::template NextContextType
                <typename tThen::ValueType, tFunc, tValueType>(std::forward<tFunc>(f));
        this->setTarget(next);
        DLOG("<< then");
        return { next };
    }

    template<typename tFunc>
    void onError(tFunc &&f)
    {
        /* Constraints for a provided callable are checked inside ErrorHandler */
        this->addErrorHandler(std::unique_ptr<ErrorHandlerNtBase>(
                                  new ErrorHandler<tValueType, tFunc>(std::forward<tFunc>(f))));
    }

    template<typename tErrorType>
    void setError(tErrorType &&error)
    {
        DLOG(">> setError");
        this->storeError(std::unique_ptr<StoredErrorNtBase>(
                             new StoredError<tErrorType>(std::forward<tErrorType>(error))));
        DLOG("<< setError");
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

template<typename ValueType>
class PromiseBase
        : public PromiseNtBase
{
public:
    using ContextType = ContextBase<ValueType>;

public:
    PromiseBase() noexcept
        : m_ctx(new InitialContext<ValueType>())
    {
    }

    ~PromiseBase() noexcept
    {
        if ( m_ctx )
        {
            //setError(-1);
        }
    }

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
