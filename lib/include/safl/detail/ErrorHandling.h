/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Local includes:
#include "Utils.h"
#include "FunctionTraits.h"

namespace safl
{
namespace detail
{

class ContextNtBase;

template<typename tValueType>
class ContextBase;

/**
 * @internal
 * @defgroup ErrorHandling Error Handling
 * @{
 */

/**
 * @internal
 * @brief The non-templated class for stored errors.
 */
class StoredErrorNtBase
        : public TypeEraser
{
public:
    StoredErrorNtBase(std::type_index typeIndex, const void *data)
        : TypeEraser(typeIndex)
        , m_data(data)
    {
    }

    const void *data() const
    {
        return m_data;
    }

private:
    const void *m_data;
};

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

/**
 * @internal
 * @brief The non-templated base for error handlers.
 */
class ErrorHandlerNtBase
        : public TypeEraser
{
    using TypeEraser::TypeEraser;

public:
    virtual ~ErrorHandlerNtBase() = default;

public:
    void acceptError(ContextNtBase *ctx, const StoredErrorNtBase *error)
    {
        privateAcceptError(ctx, error->data());
    }

private:
    virtual void privateAcceptError(ContextNtBase *ctx, const void *error) = 0;
};

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

private:
    void privateAcceptError(ContextNtBase *ctx, const void *error) override
    {
        acceptErrorImpl(static_cast<ContextType*>(ctx),
                        *reinterpret_cast<const ErrorType*>(error));
    }

    template<typename xValueType = tValueType>
    void acceptErrorImpl(typename std::enable_if_t<std::is_same<xValueType, void>::value,
                                                   ContextType> *ctx,
                         const ErrorType &error)
    {
        m_f(error);
        ctx->setValue();
    }

    template<typename xValueType = tValueType>
    void acceptErrorImpl(typename std::enable_if_t<!std::is_same<xValueType, void>::value,
                                                   ContextType> *ctx,
                         const ErrorType &error)
    {
        ctx->setValue(m_f(error));
    }

private:
    std::decay_t<tFunc> m_f;
};

/// @}

} // namespace detail
} // namespace safl
