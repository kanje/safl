/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

#include "Future.h"

#include <vector>
#include <map>

namespace safl {

namespace detail {

template<typename tPrevContext, typename tInput>
class CollectContainer
        : public std::map<tPrevContext, tInput>
{
};

template<typename tPrevContext>
class CollectContainer<tPrevContext, void>
        : public std::set<tPrevContext>
{
};

template<typename tInput>
class CollectContextBase
        : public ContextBase<std::vector<tInput>>
{
};

template<>
class CollectContextBase<void>
        : public ContextBase<void>
{
};

template<typename tInput>
class CollectContext final
        : public CollectContextBase<tInput>
{
public:
    using PrevContextType = ContextValueBase<tInput>;

public:
    CollectContext(std::vector<Future<tInput>> &futures) noexcept
        : m_expectedSize(futures.size())
    {
        DLOG(">> collect: size=" << m_expectedSize);

        if ( m_expectedSize == 0 ) {
            this->fulfil();
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

        this->insertValue(ctx);

        if ( this->isReady() ) {
            /* At lest one collected future reported an error, so this future
             * is already fulfilled with an error. The rest is ignored. */
            DLOG("input IGNORED");
            return;
        }

        if ( m_values.size() == m_expectedSize ) {
            this->fulfil();
        }
    }

    void acceptError(ContextNtBase *ctx, Signal &&error) noexcept override
    {
        (void)ctx;
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
    template<typename xInput = tInput>
    std::enable_if_t<std::is_void<xInput>::value>
    fulfil() noexcept
    {
        this->setValue();
    }

    template<typename xInput = tInput>
    std::enable_if_t<!std::is_void<xInput>::value>
    fulfil() noexcept
    {
        std::vector<xInput> result;
        result.reserve(m_expectedSize);
        for ( auto *orderedCtx : m_ctxOrder ) {
            result.push_back(m_values.at(orderedCtx));
        }
        this->setValue(std::move(result));
    }

    template<typename xInput = tInput>
    std::enable_if_t<std::is_void<xInput>::value>
    insertValue(PrevContextType *ctx) noexcept
    {
        m_values.insert(ctx);
    }

    template<typename xInput = tInput>
    std::enable_if_t<!std::is_void<xInput>::value>
    insertValue(PrevContextType *ctx) noexcept
    {
        m_values.insert(std::make_pair(ctx, ctx->value()));
    }

private:
    const std::size_t m_expectedSize;
    std::vector<PrevContextType*> m_ctxOrder;
    CollectContainer<PrevContextType*, tInput> m_values;
};

} // namespace detail

template<typename tValue>
auto collect(std::vector<Future<tValue>> &futures) noexcept
    -> Future<std::vector<tValue>>
{
    return { new detail::CollectContext<tValue>(futures) };
}

inline Future<void> collect(std::vector<Future<void>> &futures) noexcept
{
    return { new detail::CollectContext<void>(futures) };
}

} // namespace safl
