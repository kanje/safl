/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

#include "Future.h"

#include <vector>
#include <map>

namespace safl {

namespace detail {

template<typename tInput>
class CollectContext final
        : public ContextBase<std::vector<tInput>>
{
public:
    using PrevContextType = ContextValueBase<tInput>;

public:
    CollectContext(std::vector<Future<tInput>> &futures) noexcept
        : m_expectedSize(futures.size())
    {
        DLOG(">> collect: size=" << m_expectedSize);

        if ( m_expectedSize == 0 ) {
            this->setValue({});
        } else {
            m_ctxOrder.reserve(m_expectedSize);
            for ( auto &future : futures ) {
                auto *ctx = future.takeContext();
                ctx->setTarget(this, true);
                m_ctxOrder.push_back(ctx);
            }
        }

        DLOG("<< collect");
    }

    void acceptInput(ContextNtBase *ctx) noexcept override
    {
        acceptInput(static_cast<PrevContextType*>(ctx));
    }

    void acceptInput(PrevContextType *ctx) noexcept
    {
        DLOG("@@ collect input: " << ctx->alias() << " : " <<
             m_values.size() + 1 << "/" << m_expectedSize);

        m_values.insert(std::make_pair(ctx, ctx->value()));

        if ( this->isReady() ) {
            /* At lest one collected future reported an error, so this future
             * is already fulfilled with an error. The rest is ignored. */
            DLOG("input IGNORED");
            return;
        }

        if ( m_values.size() == m_expectedSize ) {
            std::vector<tInput> result;
            result.reserve(m_expectedSize);
            for ( auto *orderedCtx : m_ctxOrder ) {
                result.push_back(m_values.at(orderedCtx));
            }
            this->setValue(std::move(result));
        }
    }

    void acceptError(ContextNtBase *ctx, Signal &&error) noexcept override
    {
        DLOG("@@ collect ERROR: " << ctx->alias());
        /* Report only the first received error. Any further errors from other
         * observed futures are ignored. */
        if ( !this->isReady() ) {
            this->storeError(std::move(error));
        } else {
            DLOG("error IGNORED");
        }
    }

private:
    const std::size_t m_expectedSize;
    std::vector<PrevContextType*> m_ctxOrder;
    std::map<PrevContextType*, tInput> m_values;
};

} // namespace detail

template<typename tValue>
auto collect(std::vector<Future<tValue>> &futures) noexcept
    -> Future<std::vector<tValue>>
{
    return { new detail::CollectContext<tValue>(futures) };
}

} // namespace safl
