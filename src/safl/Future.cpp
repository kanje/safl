/*
 * This file is a part of Standalone Future Library (safl).
 */

// Self-include:
#include <safl/Future.h>

// Std includes:
#include <cassert>
#include <map>

// Other includes:
#include <QTimer>

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

ContextNtBase::ContextNtBase() noexcept
    : m_prev(nullptr)
    , m_next(nullptr)
    , m_isValueSet(false)
{
    DLOG("new");
}

ContextNtBase::~ContextNtBase() noexcept
{
    DLOG("delete");
}

void ContextNtBase::setValue() noexcept
{
    assert(m_isValueSet == false);
    if ( m_next )
    {
        fulfil();
    }
}

void ContextNtBase::setTarget(ContextNtBase *next) noexcept
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

void ContextNtBase::fulfil() noexcept
{
    DLOG("fulfil");
    QTimer::singleShot(0, [this]()
    {
        m_next->acceptInput();
        m_next->m_prev = nullptr;
    });
}
