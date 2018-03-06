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
    : m_prev(nullptr)
    , m_next(nullptr)
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
    return (m_hasPromise || (m_prev != nullptr)) && (m_hasFuture || (m_next != nullptr));
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
    assert(next->m_prev);
    m_isShadow = true;
    m_hasFuture = false;
    next->m_prev->unsetTarget();
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

void ContextNtBase::detachFuture()
{
    DLOG("detachFuture");
    assert(m_hasFuture);
    m_hasFuture = false;
    tryDestroy();
}

void ContextNtBase::setTarget(ContextNtBase *next)
{
    DLOG("setTarget: " << next->alias());
    assert(!m_next);
    assert(!next->m_prev);
    m_next = next;
    m_next->m_prev = this;
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
        assert(m_next->m_prev == this);
        m_next->m_prev = nullptr;
        m_next->tryDestroy();
        m_next = nullptr;
        tryDestroy();
    }
}

void ContextNtBase::fulfil()
{
    DLOG("fulfil");
    assert(Executor::instance() != nullptr);

    auto doFulfil = [this]()
    {
        /* m_next will not be deleted by acceptInput() because m_next->m_prev
         * is not null, so it is safe to operate on it. */
        m_next->acceptInput();

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

void ContextNtBase::storeError(UniqueStoredError &&error)
{
    assert(!m_isValueSet);
    assert(!m_storedError);

    /* Search for an appropriate error handler. */
    for ( auto &handler : m_errorHandlers ) {
        if ( tryHandleError(error, handler) ) {
            return;
        }
    }

    if ( m_next != nullptr ) {
        forwardError(std::move(error));
    } else {
        m_storedError = std::move(error);
    }
}

void ContextNtBase::forwardError(UniqueStoredError &&error)
{
    /* If there is no error handler for this context, try the next one.
     * The next context is not destroyed, because m_next->m_prev is not null. */
    m_next->storeError(std::move(error));

    /* Mark this context as fulfilled. This will make isReady() return a valid
     * value and prevent reporting a broken promise. */
    m_isErrorForwarded = true;

    /* This disconnects this and the next contexts. One or both of them might
     * be destroyed in process. */
    unsetTarget();
}

void ContextNtBase::addErrorHandler(UniqueErrorHandler &&handler)
{
    /* Maybe the new error handler can handle a stored error? */
    if ( m_storedError ) {
        tryHandleError(m_storedError, handler);
    } else {
        m_errorHandlers.push_back(std::move(handler));
    }
}

bool ContextNtBase::tryHandleError(UniqueStoredError &error, UniqueErrorHandler &handler)
{
    if ( handler->isOfTypeAs(error) ) {
        Executor::instance()->
                invoke([this, error = std::move(error), handler = std::move(handler)]()
        {
            handler->acceptError(this, error.get());
        });
        return true;
    }
    return false;
}

void ContextNtBase::tryDestroy()
{
    if ( !(m_hasPromise || m_hasFuture || (m_prev != nullptr) || (m_next != nullptr)) ) {
        delete this;
    }
}
