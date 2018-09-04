/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Self-include:
#include <safl/detail/DebugContext.h>

using namespace safl::detail;

static unsigned int s_nextAlias = 0;
static unsigned int s_cntContexts = 0;

unsigned int DebugContext::cntContexts() noexcept
{
    return s_cntContexts;
}

void DebugContext::resetCounters() noexcept
{
    s_nextAlias = 0;
    s_cntContexts = 0;
}

DebugContext::DebugContext() noexcept
    : m_alias(s_nextAlias++)
{
    DLOG("new");
    s_cntContexts++;
}

DebugContext::~DebugContext() noexcept
{
    s_cntContexts--;
    DLOG("delete");
}
