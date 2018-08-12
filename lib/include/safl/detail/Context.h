/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "DebugContext.h"
#include "Signalling.h"

// Std includes:
#include <memory>
#include <vector>

namespace safl {

template<typename tValueType>
class Future;

namespace detail {

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
#ifdef SAFL_DEVELOPER
        , protected DebugContext
#endif
{
public:
    bool isReady() const;
    bool isFulfillable() const;
    void setValue();
    void makeShadowOf(ContextNtBase *next);
    void attachPromise();
    void detachPromise();
    void attachFuture();
    void detachFuture();

public:
    template<typename tFunc>
    void onMessage(tFunc &&f)
    {
        addMessageHandler(makeMessageHandler(std::forward<tFunc>(f)));
    }

    template<typename tMessage>
    void sendMessage(tMessage &&msg)
    {
        storeMessage(makeSignal(std::forward<tMessage>(msg)));
    }

protected:
    ContextNtBase();
    virtual ~ContextNtBase();
    void setTarget(ContextNtBase *next);
    void storeError(Signal &&error);
    void addErrorHandler(SignalHandler &&handler);

private:
    void fulfil();
    void forwardError(Signal &&error);
    bool tryHandleSignal(Signal &error, SignalHandler &handler);

    void storeMessage(Signal &&msg);
    void addMessageHandler(SignalHandler &&handler);

    virtual void acceptInput(ContextNtBase *ctx) = 0;
    void unsetTarget();
    void tryDestroy();

protected:
    ContextNtBase *m_prev;
    ContextNtBase *m_next;
    bool m_isValueSet;
    bool m_isErrorForwarded;
    bool m_isShadow;
    bool m_hasFuture;
    bool m_hasPromise;

private: // error handling
    Signal m_storedError;
    std::vector<SignalHandler> m_errorHandlers;

    std::vector<Signal> m_storedMessages;
    std::vector<SignalHandler> m_messageHandlers;
};

template<typename tValueType>
class ContextValueBase
        : public ContextNtBase
{
public:
    ~ContextValueBase()
    {
        if ( m_isValueSet ) {
            reinterpret_cast<tValueType*>(&m_value)->~tValueType();
        }
    }

    const tValueType &value() const noexcept
    {
        return *reinterpret_cast<const tValueType*>(&m_value);
    }

    void setValue(const tValueType &value) noexcept
    {
        DLOG(">> setValue: " << value);
        new (&m_value) tValueType(value);
        ContextNtBase::setValue();
        DLOG("<< setValue");
    }

private:
    std::aligned_storage_t<sizeof(tValueType)> m_value;
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
                      (std::is_same<tValueType, void>::value ? 0 : 1),
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
        auto nextCtx = new typename Then::template NextContextType
                <typename Then::ValueType, tFunc, tValueType>(std::forward<tFunc>(f));
        typename Then::FutureType nextFuture(nextCtx);
        this->setTarget(nextCtx);
        DLOG("<< then");
        return nextFuture;
    }

    template<typename tFunc>
    void onError(tFunc &&f)
    {
        /* Constraints for a provided callable are checked inside ErrorHandler */
        this->addErrorHandler(makeErrorHandler<tValueType>(std::forward<tFunc>(f)));
    }

    template<typename tErrorType>
    void setError(tErrorType &&error)
    {
        DLOG(">> setError");
        this->storeError(makeSignal(std::forward<tErrorType>(error)));
        DLOG("<< setError");
    }
};

/*******************************************************************************
 * Concrete future contexts.
 */

template<typename tValue>
class InitialContext final
        : public ContextBase<tValue>
{
private:
    void acceptInput(ContextNtBase */*ctx*/) noexcept override
    {
        // do nothing
    }
};

template<typename tValue, typename tFunc, typename tInput>
class NextContextBase
        : public ContextBase<tValue>
{
public:
    NextContextBase(tFunc &&f)
        : m_f(std::forward<tFunc>(f))
    {
    }

protected:
    std::decay_t<tFunc> m_f;
};

template<bool HasValue, bool HasInput, typename xValue, typename xInput>
using CondContextType =
    std::enable_if_t<(HasValue != std::is_void<xValue>::value) &&
                     (HasInput != std::is_void<xInput>::value),
                     ContextValueBase<xInput>*>;

template<typename tValue, typename tFunc, typename tInput>
class SyncNextContext final
        : public NextContextBase<tValue, tFunc, tInput>
{
    using NextContextBase<tValue, tFunc, tInput>::NextContextBase;

private:
    void acceptInput(ContextNtBase *ctx) noexcept override
    {
        acceptInput(static_cast<ContextValueBase<tInput>*>(ctx));
    }

    template<typename xValue = tValue, typename xInput = tInput>
    void acceptInput(CondContextType<true, true, xValue, xInput> ctx) noexcept
    {
        DLOG(">> acceptInput");
        this->setValue(this->m_f(ctx->value()));
        DLOG("<< acceptInput");
    }

    template<typename xValue = tValue, typename xInput = tInput>
    void acceptInput(CondContextType<false, true, xValue, xInput> ctx) noexcept
    {
        DLOG(">> acceptInput (void output)");
        this->m_f(ctx->value());
        this->setValue();
        DLOG("<< acceptInput (void output)");
    }

    template<typename xValue = tValue, typename xInput = tInput>
    void acceptInput(CondContextType<true, false, xValue, xInput>) noexcept
    {
        DLOG(">> acceptInput (void input)");
        this->setValue(this->m_f());
        DLOG("<< acceptInput (void input)");
    }

    template<typename xValue = tValue, typename xInput = tInput>
    void acceptInput(CondContextType<false, false, xValue, xInput>) noexcept
    {
        DLOG(">> acceptInput (void input and output)");
        this->m_f();
        this->setValue();
        DLOG("<< acceptInput (void input and output)");
    }
};

template<typename tValue, typename tFunc, typename tInput>
class AsyncNextContext final
        : public NextContextBase<tValue, tFunc, tInput>
{
    using NextContextBase<tValue, tFunc, tInput>::NextContextBase;

private:
    void acceptInput(ContextNtBase *ctx) noexcept override
    {
        acceptInput(static_cast<ContextValueBase<tInput>*>(ctx));
    }

    template<typename xValue = tValue, typename xInput = tInput>
    void acceptInput(CondContextType<true, true, xValue, xInput> ctx) noexcept
    {
        if ( m_shadow == nullptr ) {
            DLOG(">> acceptInput (create shadow)");
            m_shadow = this->m_f(ctx->value()).makeShadowOf(this);
            DLOG("<< acceptInput (create shadow)");
        } else {
            DLOG(">> acceptInput (process shadow)");
            this->setValue(m_shadow->value());
            DLOG("<< acceptInput (process shadow)");
        }
    }

private:
    ContextBase<tValue> *m_shadow = nullptr;
};

} // namespace detail
} // namespace safl
