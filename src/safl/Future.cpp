/*
 * This file is a part of Standalone Future Library (safl).
 */

// Self-include:
#include <safl/Future.h>

// Std includes:
#include <cassert>
#include <map>

char mnemo(void *p)
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
    assert(m_isValueSet == false);
    m_isValueSet = true;
    if ( m_next )
    {
        fulfil();
    }
}

void ContextNtBase::makeShadowOf(ContextNtBase *next)
{
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

    auto doFulfil = [](ContextNtBase *self)
    {
        self->m_next->acceptInput();
        self->m_next->m_prev = nullptr;
    };

    if ( m_isShadowed )
    {
        doFulfil(this);
    }
    else
    {
        s_executor->invoke(this, doFulfil);
    }
}

void ContextNtBase::addErrorHandler(std::unique_ptr<ErrorHandlerNtBase> &&errorHandler)
{
    /* Maybe the new error handle can handle a stored error? */
    if ( m_storedError && m_storedError->isType(errorHandler.get()) )
    {
        errorHandler->acceptError(this, m_storedError.get());
    }
    else
    {
        m_errorHandlers.push_back(std::move(errorHandler));
    }
}

void ContextNtBase::storeError(std::unique_ptr<StoredErrorNtBase> &&error)
{
    assert(!m_storedError);
    m_storedError = std::move(error);

    for ( const auto &errorHandler : m_errorHandlers )
    {
        if ( errorHandler->isType(m_storedError.get()) )
        {
            errorHandler->acceptError(this, m_storedError.get());
            break;
        }
    }
}

ErrorHandlerNtBase::~ErrorHandlerNtBase() noexcept
{
}
