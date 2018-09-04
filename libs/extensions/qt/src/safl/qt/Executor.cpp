/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#include <safl/qt/Executor.h>

#include <safl/Executor.h>

#include <QObject>
#include <QEvent>
#include <QCoreApplication>

using namespace safl::qt;

namespace {

using safl::Executor;
using safl::detail::Task;

class SaflEvent final
        : public QEvent
{
public:
    static QEvent::Type s_eventType;

public:
    SaflEvent(Task &&f)
        : QEvent(s_eventType)
        , f(std::move(f))
    {
    }

    Task f;
};

QEvent::Type SaflEvent::s_eventType;

class QtExecutor final
        : public QObject
        , public Executor
{
public:
    QtExecutor() noexcept
    {
        SaflEvent::s_eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
    }

private:
    void invoke(Task &&f) noexcept override
    {
        QCoreApplication::postEvent(this, new SaflEvent(std::move(f)));
    }

    void customEvent(QEvent *event) override
    {
        if ( event->type() == SaflEvent::s_eventType ) {
            auto *se = static_cast<SaflEvent*>(event);
            se->f.invoke();
            se->accept();
        }
    }
};

} // anonymous namespace

ExecutorScope::ExecutorScope() noexcept
{
    static QtExecutor s_qtExecutor;
    m_oldExecutor = Executor::instance();
    Executor::setInstance(&s_qtExecutor);
}

ExecutorScope::~ExecutorScope()
{
    Executor::setInstance(m_oldExecutor);
}
