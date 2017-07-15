/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Self-include:
#include <safl/Future.h>

// Local includes:
#include <safl/detail/Executor.h>

// Std includes:
#include <cassert>
#include <map>

char safl::detail::mnemo(void *p)
{
    static std::map<void*, char> s_map;
    static char s_next = 'A';
    auto it = s_map.find(p);
    if ( it != s_map.end() )
    {
        return it->second;
    }
    s_map[p] = s_next;
    return s_next++;
}

using namespace safl::detail;

static Executor *s_executor = nullptr;

void Executor::set(Executor *executor) noexcept
{
    s_executor = executor;
}

ContextNtBase::ContextNtBase()
    : m_prev(nullptr)
    , m_next(nullptr)
    , m_isValueSet(false)
    , m_isShadowed(false)
{
    DLOG("new");
}

ContextNtBase::~ContextNtBase()
{
    DLOG("delete");
}

void ContextNtBase::setValue()
{
    assert(!m_isValueSet);
    assert(!m_storedError);
    m_isValueSet = true;
    if ( m_next )
    {
        fulfil();
    }
}

void ContextNtBase::makeShadowOf(ContextNtBase *next)
{
    assert(!m_isShadowed);
    m_isShadowed = true;
    setTarget(next);
}

void ContextNtBase::setTarget(ContextNtBase *next)
{
    DLOG("setTarget: " << mnemo(next));
    assert(m_next == nullptr);
    m_next = next;
    m_next->m_prev = this;
    if ( m_isValueSet )
    {
        fulfil();
    }
}

void ContextNtBase::fulfil()
{
    DLOG("fulfil");
    assert(s_executor != nullptr);

    auto doFulfil = [this]()
    {
        m_next->acceptInput();
        m_next->m_prev = nullptr;
    };

    if ( m_isShadowed )
    {
        doFulfil();
    }
    else
    {
        s_executor->invoke(std::move(doFulfil));
    }
}

void ContextNtBase::storeError(UniqueStoredError &&error)
{
    assert(!m_isValueSet);
    assert(!m_storedError);

    /* Search for an appropriate error handler. */
    for ( auto &handler : m_errorHandlers )
    {
        if ( tryHandleError(error, handler) )
        {
            return;
        }
    }

    if ( m_next )
    {
        /* If there is no error handler for this context, try the next one.
         * This fulfills this context and it must be destroyed. */
        m_next->storeError(std::move(error));
        delete this;
    }
    else
    {
        m_storedError = std::move(error);
    }
}

void ContextNtBase::addErrorHandler(UniqueErrorHandler &&handler)
{
    /* Maybe the new error handler can handle a stored error? */
    if ( m_storedError )
    {
        tryHandleError(m_storedError, handler);
    }
    else
    {
        m_errorHandlers.push_back(std::move(handler));
    }
}

bool ContextNtBase::tryHandleError(UniqueStoredError &error, UniqueErrorHandler &handler)
{
    if ( handler->isType(error) )
    {
        s_executor->invoke([this, error = std::move(error), handler = std::move(handler)]()
        {
            handler->acceptError(this, error.get());
        });
        return true;
    }
    return false;
}
