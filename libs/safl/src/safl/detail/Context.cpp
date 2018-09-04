/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Self-include:
#include <safl/detail/Context.h>

// Local includes:
#include <safl/Executor.h>

// Std includes:
#include <cassert>

using namespace safl::detail;

ContextNtBase::ContextNtBase()
    : m_next(nullptr)
    , m_isValueSet(false)
    , m_isErrorForwarded(false)
    , m_isShadow(false)
    , m_hasFuture(false)
    , m_hasPromise(false)
{
}

ContextNtBase::~ContextNtBase() = default;

bool ContextNtBase::isReady() const
{
    return m_isValueSet || m_storedError || m_isErrorForwarded;
}

bool ContextNtBase::isFulfillable() const
{
    /* The context is fulfillable if both a result can be achieved (e.g. a value
     * can be set by a previous context or a Promise), and the result can be used
     * (i.e. it can be propagated to the next context or accessed via a Future). */
    return (m_hasPromise || m_prev.empty()) && (m_hasFuture || (m_next != nullptr));
}

void ContextNtBase::setValue()
{
    assert(!m_isValueSet);
    assert(!m_storedError);
    m_isValueSet = true;
    if ( m_next != nullptr ) {
        fulfil();
    }
}

void ContextNtBase::makeShadowOf(ContextNtBase *next)
{
    DLOG("makeShadowOf: " << next->alias());
    assert(!m_isShadow);
    assert(m_hasFuture);
    assert(next->m_prev.size() == 1);
    m_isShadow = true;
    m_hasFuture = false;
    (*next->m_prev.begin())->unsetTarget();
    setTarget(next);
}

void ContextNtBase::attachPromise()
{
    DLOG("attachPromise");
    assert(!m_hasPromise);
    m_hasPromise = true;
}

void ContextNtBase::detachPromise()
{
    DLOG("detachPromise");
    assert(m_hasPromise);
    m_hasPromise = false;
    tryDestroy();
}

void ContextNtBase::attachFuture()
{
    DLOG("attachFuture");
    assert(!m_hasFuture);
    m_hasFuture = true;
}

void ContextNtBase::detachFuture(bool doTryDestroy)
{
    DLOG("detachFuture");
    assert(m_hasFuture);
    m_hasFuture = false;
    if ( doTryDestroy ) {
        tryDestroy();
    }
}

void ContextNtBase::setTarget(ContextNtBase *next, bool doMakeDirect)
{
    DLOG("setTarget: " << next->alias() <<
         (m_isShadow || doMakeDirect ? " (direct)" : ""));
    assert(!m_next);
    assert(next->m_prev.count(this) == 0);
    m_next = next;
    m_next->m_prev.insert(this);
    m_isShadow = m_isShadow || doMakeDirect;
    if ( m_isValueSet ) {
        fulfil();
    }
    if ( m_storedError ) {
        forwardError(std::move(m_storedError));
    }
}

void ContextNtBase::unsetTarget()
{
    if ( m_next != nullptr ) {
        DLOG("unsetTarget: " << m_next->alias());
        assert(m_next->m_prev.count(this) == 1);
        m_next->m_prev.erase(this);
        m_next->tryDestroy();
        m_next = nullptr;
        tryDestroy();
    }
}

void ContextNtBase::fulfil()
{
    DLOG("fulfil" << (m_isShadow ? " (direct)" : ""));
    assert(Executor::instance() != nullptr);

    auto doFulfil = [this]()
    {
        /* m_next will not be deleted by acceptInput() because m_next->m_prev
         * is not null, so it is safe to operate on it. */
        m_next->acceptInput(this);

        /* This disconnects this and the next contexts. One or both of them might
         * be destroyed in process. */
        unsetTarget();
    };

    if ( m_isShadow ) {
        doFulfil();
    } else {
        Executor::instance()->invoke(std::move(doFulfil));
    }
}

void ContextNtBase::storeError(Signal &&error)
{
    assert(!m_isValueSet);
    assert(!m_storedError);

    /* Search for an appropriate error handler. */
    if ( tryHandleSignal(error, m_errorHandlers) ) {
        return;
    }

    if ( m_next != nullptr ) {
        forwardError(std::move(error));
    } else {
        m_storedError = std::move(error);
    }
}

void ContextNtBase::forwardError(Signal &&error)
{
    /* If there is no error handler for this context, try the next one.
     * The next context is not destroyed, because m_next->m_prev is not null. */
    m_next->acceptError(this, std::move(error));

    /* Mark this context as fulfilled. This will make isReady() return a valid
     * value and prevent reporting a broken promise. */
    m_isErrorForwarded = true;

    /* This disconnects this and the next contexts. One or both of them might
     * be destroyed in process. */
    unsetTarget();
}

void ContextNtBase::addErrorHandler(SignalHandler &&handler)
{
    /* Maybe the new error handler can handle a stored error? */
    if ( m_storedError ) {
        tryHandleSignal(m_storedError, handler);
    } else {
        m_errorHandlers.push_back(std::move(handler));
    }
}

bool ContextNtBase::tryHandleSignal(Signal &sig, SignalHandler &handler)
{
    if ( handler->isOfTypeAs(sig) ) {
        Executor::instance()->
                invoke([this, sig = std::move(sig), handler = std::move(handler)]()
        {
            handler->accept(this, sig.get());
        });
        return true;
    }
    return false;
}

bool ContextNtBase::tryHandleSignal(
        Signal &sig, std::vector<SignalHandler> &handlers)
{
    for ( auto &handler : handlers ) {
        if ( tryHandleSignal(sig, handler) ) {
            return true;
        }
    }
    return false;
}

void ContextNtBase::acceptMessage(Signal &&msg) noexcept
{
    /* The message must be sent to the currently running context. */
    if ( m_prev.size() == 1 ) {
        /* Handle the most common case. */
        (*m_prev.begin())->acceptMessage(std::move(msg));
    } else {
        for ( auto *prev : m_prev ) {
            prev->acceptMessage(msg->clone());
        }
    }
}

void ContextNtBase::addMessageHandler(SignalHandler &&/*handler*/)
{
}

void ContextNtBase::acceptError(ContextNtBase *ctx, Signal &&error) noexcept
{
    assert(m_prev.count(ctx) == 1);
    storeError(std::move(error));
}

void ContextNtBase::acceptInput(ContextNtBase */*ctx*/)
{
}

void ContextNtBase::tryDestroy()
{
    if ( !(m_hasPromise || m_hasFuture || !m_prev.empty() || (m_next != nullptr)) ) {
        delete this;
    }
}
