#ifndef LayoutGlobalScope_h
#define LayoutGlobalScope_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ScriptValue.h"
#include "bindings/core/v8/LayoutScriptController.h"
#include "core/dom/ExecutionContext.h"
#include "core/dom/ExecutionContextTask.h"
#include "core/events/EventTarget.h"
#include "core/frame/DOMTimer.h"
#include "core/frame/DOMTimerCoordinator.h"
#include "core/frame/DOMWindowBase64.h"
#include "core/workers/WorkerEventQueue.h"
#include "platform/heap/Handle.h"
#include "wtf/OwnPtr.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefCounted.h"
#include "wtf/RefPtr.h"

namespace blink {

class MainThreadTaskRunner;

class LayoutGlobalScope : public EventTargetWithInlineData, public RefCountedWillBeNoBase<LayoutGlobalScope>, public SecurityContext, public ExecutionContext, public WillBeHeapSupplementable<LayoutGlobalScope>, public DOMWindowBase64 {
    DEFINE_WRAPPERTYPEINFO();
    REFCOUNTED_EVENT_TARGET(LayoutGlobalScope);
    WILL_BE_USING_GARBAGE_COLLECTED_MIXIN(LayoutGlobalScope);
public:
    // HACK
    // using RefCounted<LayoutGlobalScope>::ref;
    // using RefCounted<LayoutGlobalScope>::deref;

    static PassRefPtrWillBeRawPtr<LayoutGlobalScope> create(const KURL&, const String&);
    virtual ~LayoutGlobalScope();

    // LayoutGlobalScope
    LayoutGlobalScope* self() { return this; }
    const AtomicString& tmp() const;

    ScriptValue fn() const { return m_fnValue; }
    void setFn(ScriptValue& fnValue) { m_fnValue = fnValue; }

    // EventTarget
    virtual const AtomicString& interfaceName() const override;

    // ExecutionContext
    virtual String userAgent(const KURL&) const override final;
    virtual void disableEval(const String& errorMessage) override final;
    virtual ExecutionContext* executionContext() const override final;
    virtual void postTask(PassOwnPtr<ExecutionContextTask> task) override final;

    // Hack. Just WTF_LOG these.
    virtual void reportBlockedScriptExecutionToInspector(const String&) override final;
    virtual void addConsoleMessage(PassRefPtrWillBeRawPtr<ConsoleMessage>) override final;
    virtual void logExceptionToConsole(const String& errorMessage, int scriptId, const String& sourceURL, int lineNumber, int columnNumber, PassRefPtrWillBeRawPtr<ScriptCallStack>) override final;

    KURL completeURL(const String&) const;

    // ExecutionContextClient
    virtual WorkerEventQueue* eventQueue() const override final;
    virtual SecurityContext& securityContext() override final { return *this; }

    virtual bool isContextThread() const override final;
    // TODO virtual bool isJSExecutionForbidden() const override final;

    virtual double timerAlignmentInterval() const override final;
    virtual DOMTimerCoordinator* timers() override final;

    virtual bool isLayoutGlobalScope() const override final { return true; }

    LayoutScriptController* script() { return m_script.get(); }

    virtual void trace(Visitor*) override;
    void dispose();

private:
#if !ENABLE(OILPAN)
    virtual void refExecutionContext() override final { ref(); }
    virtual void derefExecutionContext() override final { deref(); }
#endif

    KURL m_url;
    String m_userAgent;
    OwnPtr<LayoutScriptController> m_script;
    OwnPtrWillBeMember<WorkerEventQueue> m_eventQueue;
    DOMTimerCoordinator m_timers;
    OwnPtr<MainThreadTaskRunner> m_taskRunner;
    ScriptValue m_fnValue;

    // ExecutionContext
    virtual const KURL& virtualURL() const override final;
    virtual KURL virtualCompleteURL(const String&) const override final;
    virtual void didUpdateSecurityOrigin() override final { }
    virtual EventTarget* errorEventTarget() override final;

    LayoutGlobalScope(const KURL&, const String&);
};

DEFINE_TYPE_CASTS(LayoutGlobalScope, ExecutionContext, context, context->isLayoutGlobalScope(), context.isLayoutGlobalScope());

} // namespace blink

#endif // LayoutGlobalScope_h
