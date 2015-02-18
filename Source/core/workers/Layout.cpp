#include "config.h"
#include "core/workers/Layout.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/V8PerIsolateData.h"
#include "bindings/core/v8/ScriptSourceCode.h"
#include "bindings/core/v8/ToV8.h"
#include "core/events/MessageEvent.h"
#include "core/workers/LayoutArgs.h"
#include "core/workers/WorkerScriptLoader.h"
#include "platform/Logging.h"
#include "wtf/StdLibExtras.h"
#include <v8.h>

namespace blink {

inline Layout::Layout(ExecutionContext* context)
    : AbstractWorker(context)
{
}

PassRefPtrWillBeRawPtr<Layout> Layout::create(ExecutionContext* context, const String& url, ExceptionState& exceptionState)
{
    ASSERT(isMainThread());

    RefPtrWillBeRawPtr<Layout> layout = adoptRefWillBeNoop(new Layout(context));

    layout->suspendIfNeeded();

    // Probably want to throw here instead?
    KURL scriptURL = layout->resolveURL(url, exceptionState);
    if (scriptURL.isEmpty())
        return nullptr;

    layout->m_scriptLoader = WorkerScriptLoader::create();
    layout->m_scriptLoader->loadAsynchronously(*context, scriptURL, DenyCrossOriginRequests, layout.get());
    // TODO create LayoutScope object.
    return layout.release();
}

Layout::~Layout()
{
}

const AtomicString& Layout::type() const
{
    DEFINE_STATIC_LOCAL(AtomicString, type, ("foo", AtomicString::ConstructFromLiteral));
    return type;
}

double Layout::invoke(double arg1, double arg2) const
{
    RefPtr<LayoutArgs> args = LayoutArgs::create(arg1, arg2);
    v8::Isolate* isolate = m_layoutGlobalScope->script()->isolate();

    m_layoutGlobalScope->script()->context()->Enter();

    v8::Handle<v8::Value> argsHandle = toV8(args.get(), m_layoutGlobalScope->script()->context()->Global(), isolate);
    v8::Handle<v8::Value> argv[] = { argsHandle };
    v8::Handle<v8::Function> fn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->fn().v8Value());
    v8::Handle<v8::Value> result = m_layoutGlobalScope->script()->callFunction(fn, v8::Null(isolate), 1, argv);
    double ret = v8::Handle<v8::Number>::Cast(result)->Value();
    m_layoutGlobalScope->script()->context()->Exit();
    return ret;
}

const AtomicString& Layout::interfaceName() const
{
    return EventTargetNames::Layout;
}

void Layout::didReceiveResponse(unsigned long identifier, const ResourceResponse&)
{
    // InspectorInstrumentation::didReceiveScriptResponse(executionContext(), identifier);
    WTF_LOG(NotYetImplemented, "didReceiveResponse");
}

void Layout::notifyFinished()
{
    if (m_scriptLoader->failed()) {
        dispatchEvent(Event::createCancelable(EventTypeNames::error));
    } else {
        //ASSERT(m_contextProxy);
        //WorkerThreadStartMode startMode = DontPauseWorkerGlobalScopeOnStart;
        //if (InspectorInstrumentation::shouldPauseDedicatedWorkerOnStart(executionContext()))
        //    startMode = PauseWorkerGlobalScopeOnStart;
        //m_contextProxy->startWorkerGlobalScope(m_scriptLoader->url(), executionContext()->userAgent(m_scriptLoader->url()), m_scriptLoader->script(), startMode);
        //InspectorInstrumentation::scriptImported(executionContext(), m_scriptLoader->identifier(), m_scriptLoader->script());
        WTF_LOG(NotYetImplemented, "%s\n", m_scriptLoader->script().ascii().data());
        //overrideMainThread(true);
        //m_isolate = V8PerIsolateData::initializeSameThread();
        //m_isolate->Exit();
        //overrideMainThread(false);
        m_layoutGlobalScope = LayoutGlobalScope::create(m_scriptLoader->url(), executionContext()->userAgent(m_scriptLoader->url()));
        m_layoutGlobalScope->script()->evaluate(ScriptSourceCode(m_scriptLoader->script(), m_scriptLoader->url()));
    }
    m_scriptLoader = nullptr;
}


void Layout::trace(Visitor* visitor)
{
    AbstractWorker::trace(visitor);
}

} // namespace blink
