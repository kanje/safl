/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "ErrorHandling.h"

// Std includes:
#include <memory>
#include <vector>

#ifdef SAFL_DEVELOPER
#define DLOG(__message) do {                                                   \
    std::cout << "[safl] " << this->alias() << ": " << __message << std::endl; \
} while ( !42 )
#else
#define DLOG(__message) do { /* no-op */ } while ( !42 )
#endif

namespace safl
{

template<typename tValueType>
class Future;

namespace detail
{

class FutureNtBase;

template<typename tValueType, typename tFunc, typename tInputType>
class SyncNextContext;

template<typename tValueType, typename tFunc, typename tInputType>
class AsyncNextContext;

/*******************************************************************************
 * Base classes for contexts.
 */

class ContextNtBase
        : private UniqueInstance
{
    using UniqueStoredError = std::unique_ptr<StoredErrorNtBase>;
    using UniqueErrorHandler = std::unique_ptr<ErrorHandlerNtBase>;

public:
    bool isReady() const;
    bool isFulfillable() const;
    void setValue();
    void makeShadowOf(ContextNtBase *next);
    void attachPromise();
    void detachPromise();
    void attachFuture();
    void detachFuture();

#ifdef SAFL_DEVELOPER
    unsigned alias() const
    {
        return m_alias;
    }
    static std::size_t cntContexts();
#endif

protected:
    ContextNtBase();
    virtual ~ContextNtBase();
    void setTarget(ContextNtBase *next);
    void storeError(UniqueStoredError &&error);
    void addErrorHandler(UniqueErrorHandler &&handler);

private:
    void fulfil();
    bool tryHandleError(UniqueStoredError &error, UniqueErrorHandler &handler);
    virtual void acceptInput() = 0;
    void unsetTarget();
    void tryDestroy();

protected:
    ContextNtBase *m_prev;
    ContextNtBase *m_next;
    bool m_isValueSet;
    bool m_isShadow;
    bool m_hasFuture;
    bool m_hasPromise;

private: // error handling
    UniqueStoredError m_storedError;
    std::vector<UniqueErrorHandler> m_errorHandlers;

#ifdef SAFL_DEVELOPER
    unsigned m_alias;
#endif
};

template<typename tValueType>
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

    const tValueType &value() const noexcept
    {
        return *reinterpret_cast<const tValueType*>(m_value);
    }

    void setValue(const tValueType &value) noexcept
    {
        DLOG(">> setValue: " << value);
        new (&m_value) tValueType(value);
        ContextNtBase::setValue();
        DLOG("<< setValue");
    }

private:
    char m_value[sizeof(tValueType)];
};

template<>
class ContextValueBase<void>
        : public ContextNtBase
{
};

template<typename tValueType>
class ContextBase
        : public ContextValueBase<tValueType>
{
    template<typename tFunc>
    struct ThenTraits
    {
        using Traits = FunctionTraits<tFunc>;

        static_assert(Traits::NrArgs::value ==
                      std::is_same<tValueType, void>::value ? 0 : 1,
                      "then() handler for a void Future must not accept arguments, "
                      "and for other types of Future it must accept exactly one argument");

        /* If tFunc() returns Future<X>, then() also returns Future<X>.
         * Else, if tFunc() returns X, then() still returns Future<X>. */
        using FuncReturnType = typename Traits::ReturnType;
        using DoesFuncReturnFuture = std::is_base_of<FutureNtBase, FuncReturnType>;
        using FutureType = std::conditional_t<DoesFuncReturnFuture::value,
                                              FuncReturnType, Future<FuncReturnType>>;
        using ValueType = typename FutureType::ValueType;

        /* Synchronous (i.e. returning X) and asynchronous (i.e. returning Future<X>)
         * callables must be handled differently. */
        template<typename xValueType, typename xFunc, typename xInputType>
        using NextContextType = std::conditional_t<DoesFuncReturnFuture::value,
                                                   AsyncNextContext<xValueType, xFunc, xInputType>,
                                                   SyncNextContext<xValueType, xFunc, xInputType>>;
    };

public:
    template<typename tFunc>
    auto then(tFunc &&f)
    {
        using Then = ThenTraits<tFunc>;

        DLOG(">> then");
        auto next = new typename Then::template NextContextType
                <typename Then::ValueType, tFunc, tValueType>(std::forward<tFunc>(f));
        this->setTarget(next);
        DLOG("<< then");
        return typename Then::FutureType(next);
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

template<typename tValueType>
class InitialContext final
        : public ContextBase<tValueType>
{
private:
    void acceptInput() noexcept override
    {
        // do nothing
    }
};

template<typename tValueType, typename tFunc, typename tInputType>
class NextContextBase
        : public ContextBase<tValueType>
{
public:
    NextContextBase(tFunc &&f)
        : m_f(std::forward<tFunc>(f))
    {
    }

protected:
    auto prev() const noexcept
    {
        return static_cast<ContextValueBase<tInputType>*>(this->m_prev);
    }

protected:
    std::decay_t<tFunc> m_f;
};

template<typename tValueType, typename tFunc, typename tInputType>
class SyncNextContext final
        : public NextContextBase<tValueType, tFunc, tInputType>
{
    using NextContextBase<tValueType, tFunc, tInputType>::NextContextBase;

private:
    void acceptInput() noexcept override
    {
        DLOG(">> acceptInput");
        this->setValue(this->m_f(this->prev()->value()));
        DLOG("<< acceptInput");
    }
};

template<typename tFunc, typename tInputType>
class SyncNextContext<void, tFunc, tInputType> final
        : public NextContextBase<void, tFunc, tInputType>
{
    using NextContextBase<void, tFunc, tInputType>::NextContextBase;

private:
    void acceptInput() noexcept override
    {
        DLOG(">> acceptInput (void output)");
        this->m_f(this->prev()->value());
        this->setValue();
        DLOG("<< acceptInput (void output)");
    }
};

template<typename tValueType, typename tFunc>
class SyncNextContext<tValueType, tFunc, void> final
        : public NextContextBase<tValueType, tFunc, void>
{
    using NextContextBase<tValueType, tFunc, void>::NextContextBase;

private:
    void acceptInput() noexcept override
    {
        DLOG(">> acceptInput (void input)");
        this->setValue(this->m_f());
        DLOG("<< acceptInput (void input)");
    }
};

template<typename tFunc>
class SyncNextContext<void, tFunc, void> final
        : public NextContextBase<void, tFunc, void>
{
    using NextContextBase<void, tFunc, void>::NextContextBase;

private:
    void acceptInput() noexcept override
    {
        DLOG(">> acceptInput (void input and output)");
        this->m_f();
        this->setValue();
        DLOG("<< acceptInput (void input and output)");
    }
};

template<typename tValueType, typename tFunc, typename tInputType>
class AsyncNextContext final
        : public NextContextBase<tValueType, tFunc, tInputType>
{
    using NextContextBase<tValueType, tFunc, tInputType>::NextContextBase;

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
    ContextBase<tValueType> *m_shadow = nullptr;
};

} // namespace detail
} // namespace safl
