/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "FunctionTraits.h"
#include "TypeEraser.h"
#include "UniqueInstance.h"

namespace safl {
namespace detail {

class ContextNtBase;

template<typename tValueType>
class ContextBase;

/**
 * @internal
 * @defgroup ErrorHandling Error Handling
 * @{
 */

class SignalNtBase
        : public TypeEraser
        , private UniqueInstance
{
    using TypeEraser::TypeEraser;

public:
    virtual ~SignalNtBase() = default;
    virtual std::unique_ptr<SignalNtBase> clone() const noexcept = 0;
};

template<typename tData>
class SignalImpl final
        : public SignalNtBase
{
    static_assert(!std::is_reference<tData>::value,
                  "data type must not be a reference");

public:
    template<typename xData>
    explicit SignalImpl(xData &&data)
        : SignalNtBase(typeid(xData))
        , m_data(std::forward<xData>(data))
    {
    }

    const tData &data() const noexcept
    {
        return m_data;
    }

    std::unique_ptr<SignalNtBase> clone() const noexcept override
    {
        return std::make_unique<SignalImpl<tData>>(m_data);
    }

private:
    tData m_data;
};

using Signal = std::unique_ptr<SignalNtBase>;

template<typename tData>
Signal makeSignal(tData &&data)
{
    return std::make_unique<
            SignalImpl<std::remove_reference_t<tData>>>(
                std::forward<tData>(data));
}

class SignalHandlerNtBase
        : public TypeEraser
        , private UniqueInstance
{
    using TypeEraser::TypeEraser;

public:
    virtual ~SignalHandlerNtBase() = default;
    virtual void accept(ContextNtBase *ctx, const SignalNtBase *sig) = 0;
};

using SignalHandler = std::unique_ptr<SignalHandlerNtBase>;

template<typename tFunc>
class MessageHandler
        : public SignalHandlerNtBase
{
    using Traits = FunctionTraits<tFunc>;
    static_assert(Traits::NrArgs::value == 1,
                  "message handler must have exactly one argument");
    static_assert(std::is_void<typename Traits::ReturnType>::value,
                  "message handler must not return anything");

public:
    using MessageType = typename Traits::FirstArg;

public:
    explicit MessageHandler(tFunc &&f)
        : SignalHandlerNtBase(typeid(MessageType))
        , m_f(std::forward<tFunc>(f))
    {
    }

    void accept(ContextNtBase */*ctx*/, const SignalNtBase *sig) override
    {
        m_f(static_cast<const SignalImpl<MessageType>*>(sig)->data());
    }

private:
    std::decay_t<tFunc> m_f;
};

template<typename tFunc>
SignalHandler makeMessageHandler(tFunc &&f)
{
    return std::make_unique<MessageHandler<tFunc>>(std::forward<tFunc>(f));
}

template<typename tValueType, typename tFunc>
class ErrorHandler final
        : public SignalHandlerNtBase
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
        : SignalHandlerNtBase(typeid(ErrorType))
        , m_f(std::forward<tFunc>(f))
    {
    }

public:
    void accept(ContextNtBase *ctx, const SignalNtBase *sig) override
    {
        acceptError(static_cast<ContextType*>(ctx),
                    static_cast<const SignalImpl<ErrorType>*>(sig)->data());
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
    std::decay_t<tFunc> m_f;
};

template<typename tValueType, typename tFunc>
SignalHandler makeErrorHandler(tFunc &&f)
{
    return std::make_unique<ErrorHandler<tValueType, tFunc>>(std::forward<tFunc>(f));
}

/// @}

} // namespace detail
} // namespace safl
