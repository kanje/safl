/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Self-include:
#include <safl/qt/Executor.h>

// Local includes:
#include <safl/Executor.h>

// Other includes:
#include <QObject>
#include <QEvent>
#include <QCoreApplication>

namespace
{

using safl::Executor;
using safl::Invokable;

class SaflEvent final
        : public QEvent
{
public:
    static QEvent::Type s_eventType;

public:
    SaflEvent(Invokable &&f)
        : QEvent(s_eventType)
        , f(std::move(f))
    {
    }

    Invokable f;
};

class QtExecutor final
        : public QObject
        , public Executor
{
public:
    QtExecutor()
    {
        SaflEvent::s_eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
    }

private:
    void invoke(Invokable &&f) noexcept override
    {
        QCoreApplication::postEvent(this, new SaflEvent(std::move(f)));
    }

    void customEvent(QEvent *event)
    {
        if ( event->type() == SaflEvent::s_eventType )
        {
            SaflEvent *se = static_cast<SaflEvent*>(event);
            se->f.invoke();
            se->accept();
        }
    }
};

} // anonymous namespace

void safl::qt::useQtEventLoop()
{
    static QtExecutor s_qtExecutor;
    safl::Executor::set(&s_qtExecutor);
}
