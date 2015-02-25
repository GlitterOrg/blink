#include "config.h"
#include "core/customlayout/LayoutGlobalScope.h"

#include "bindings/core/v8/LayoutScriptController.h"
#include "core/dom/MainThreadTaskRunner.h"
#include "core/frame/DOMTimerCoordinator.h"
#include "core/inspector/ConsoleMessage.h"
#include "platform/Logging.h"

namespace blink {

LayoutGlobalScope::LayoutGlobalScope(const KURL& url, const String& userAgent)
    : m_url(url)
    , m_userAgent(userAgent)
    , m_script(adoptPtr(new LayoutScriptController(*this)))
    , m_eventQueue(WorkerEventQueue::create(this))
    , m_taskRunner(MainThreadTaskRunner::create(this))
{
}

void LayoutGlobalScope::dispose()
{
    m_eventQueue->close();
    ExecutionContext::notifyContextDestroyed();
}

LayoutGlobalScope::~LayoutGlobalScope()
{
}

PassRefPtrWillBeRawPtr<LayoutGlobalScope> LayoutGlobalScope::create(const KURL& url, const String& userAgent)
{
    RefPtrWillBeRawPtr<LayoutGlobalScope> context = adoptRefWillBeNoop(new LayoutGlobalScope(url, userAgent));
    return context.release();
}

void LayoutGlobalScope::postTask(PassOwnPtr<ExecutionContextTask> task)
{
    m_taskRunner->postTask(task);
}

ExecutionContext* LayoutGlobalScope::executionContext() const
{
    return const_cast<LayoutGlobalScope*>(this);
}

const AtomicString& LayoutGlobalScope::interfaceName() const
{
    return EventTargetNames::LayoutGlobalScope;
}

EventTarget* LayoutGlobalScope::errorEventTarget()
{
    return this;
}

WorkerEventQueue* LayoutGlobalScope::eventQueue() const
{
    return m_eventQueue.get();
}

bool LayoutGlobalScope::isContextThread() const
{
    return isMainThread();
}

const KURL& LayoutGlobalScope::virtualURL() const
{
    return m_url;
}

KURL LayoutGlobalScope::virtualCompleteURL(const String& url) const
{
    return completeURL(url);
}

KURL LayoutGlobalScope::completeURL(const String& url) const
{
    // Always return a null URL when passed a null string.
    // FIXME: Should we change the KURL constructor to have this behavior?
    if (url.isNull())
        return KURL();
    // Always use UTF-8 in Workers.
    return KURL(m_url, url);
}

String LayoutGlobalScope::userAgent(const KURL&) const
{
    return m_userAgent;
}

void LayoutGlobalScope::disableEval(const String& errorMessage)
{
    // m_script->disableEval(errorMessage);
    // XXX FIX.
}

double LayoutGlobalScope::timerAlignmentInterval() const
{
    return DOMTimer::visiblePageAlignmentInterval();
}

DOMTimerCoordinator* LayoutGlobalScope::timers()
{
    return &m_timers;
}

void LayoutGlobalScope::reportBlockedScriptExecutionToInspector(const String& directiveText)
{
    WTF_LOG(NotYetImplemented, "reportBlockedScriptExecutionToInspector: %s\n", directiveText.ascii().data());
}

void LayoutGlobalScope::addConsoleMessage(PassRefPtrWillBeRawPtr<ConsoleMessage> consoleMessage)
{
    WTF_LOG(NotYetImplemented, "addConsoleMessage: %s\n", consoleMessage->message().ascii().data());
}

void LayoutGlobalScope::logExceptionToConsole(const String& errorMessage, int scriptId, const String& sourceURL, int lineNumber, int columnNumber, PassRefPtrWillBeRawPtr<ScriptCallStack>)
{
    WTF_LOG(NotYetImplemented, "logExceptionToConsole: %s\n", errorMessage.ascii().data());
}

void LayoutGlobalScope::trace(Visitor* visitor)
{
/*#if ENABLE(OILPAN)
    visitor->trace(m_console);
    visitor->trace(m_location);
    visitor->trace(m_navigator);
    visitor->trace(m_workerInspectorController);
    visitor->trace(m_eventQueue);
    visitor->trace(m_workerClients);
    visitor->trace(m_timers);
    visitor->trace(m_messageStorage);
    visitor->trace(m_pendingMessages);
    HeapSupplementable<WorkerGlobalScope>::trace(visitor);
#endif*/
    ExecutionContext::trace(visitor);
    EventTargetWithInlineData::trace(visitor);
}


} // namespace blink
