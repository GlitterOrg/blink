#include "config.h"
#include "core/customlayout/LayoutWorker.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/V8PerIsolateData.h"
#include "bindings/core/v8/ScriptSourceCode.h"
#include "bindings/core/v8/ScriptState.h"
#include "bindings/core/v8/ToV8.h"
#include "core/events/MessageEvent.h"
#include "core/customlayout/LayoutArgs.h"
#include "core/customlayout/LayoutNode.h"
#include "core/rendering/RenderBox.h"
#include "core/workers/WorkerScriptLoader.h"
#include "platform/LayoutUnit.h"
#include "platform/Logging.h"
#include "wtf/StdLibExtras.h"
#include <v8.h>

namespace blink {

inline LayoutWorker::LayoutWorker(ExecutionContext* context)
    : AbstractWorker(context)
{
}

PassRefPtrWillBeRawPtr<LayoutWorker> LayoutWorker::create(ExecutionContext* context, const String& url, ExceptionState& exceptionState)
{
    ASSERT(isMainThread());

    RefPtrWillBeRawPtr<LayoutWorker> layout = adoptRefWillBeNoop(new LayoutWorker(context));

    layout->suspendIfNeeded();

    // Probably want to throw here instead?
    KURL scriptURL = layout->resolveURL(url, exceptionState);
    if (scriptURL.isEmpty())
        return nullptr;

    layout->m_scriptLoader = WorkerScriptLoader::create();
    layout->m_scriptLoader->loadAsynchronously(*context, scriptURL, DenyCrossOriginRequests, layout.get());
    return layout.release();
}

LayoutWorker::~LayoutWorker()
{
}

const AtomicString& LayoutWorker::type() const
{
    DEFINE_STATIC_LOCAL(AtomicString, type, ("foo", AtomicString::ConstructFromLiteral));
    return type;
}

double LayoutWorker::invoke(double arg1, double arg2) const
{
    ScriptState::Scope scope(m_layoutGlobalScope->fn().scriptState());

    RefPtr<LayoutArgs> args = LayoutArgs::create(arg1, arg2);
    v8::Isolate* isolate = m_layoutGlobalScope->script()->isolate();
    LayoutScriptController* script = m_layoutGlobalScope->script();

    v8::Handle<v8::Value> argsHandle = toV8(args.get(), script->context()->Global(), isolate);
    v8::Handle<v8::Value> argv[] = { argsHandle };
    v8::Handle<v8::Function> fn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->fn().v8Value());
    v8::Handle<v8::Value> result = script->callFunction(fn, v8::Null(isolate), 1, argv);

    return v8::Handle<v8::Number>::Cast(result)->Value();
}

void LayoutWorker::computeIntrinsicLogicalWidths(LayoutUnit& minLogicalWidth, LayoutUnit& maxLogicalWidth, Vector<RenderBox*>& children)
{
    ScriptState::Scope scope(m_layoutGlobalScope->calculateMinContentInlineSize().scriptState());
    LayoutScriptController* script = m_layoutGlobalScope->script();
    v8::Isolate* isolate = script->isolate();

    v8::Handle<v8::Array> arr = v8::Array::New(isolate, children.size());

    for (size_t i = 0; i < children.size(); ++i) {
        RenderBox* child = children[i];

        // Create a LayoutNode wrapper for each child, if it doesn't exist already.
        if (!child->hasScriptLayoutNode())
            child->setScriptLayoutNode(LayoutNode::create(child));

        // NOTE make this a persistent handle on RenderBoxRareData? How
        // expensive is toV8? I think ScriptWrappable contains m_wrapper which
        // is persistent.
        v8::Handle<v8::Value> node = toV8(child->scriptLayoutNode(), script->context()->Global(), isolate);

        arr->Set(i, node);
    }

    v8::Handle<v8::Value> argv[] = { arr };
    v8::Handle<v8::Function> minFn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->calculateMinContentInlineSize().v8Value());
    v8::Handle<v8::Value> minResult = script->callFunction(minFn, v8::Null(isolate), 1, argv);
    minLogicalWidth = LayoutUnit(v8::Handle<v8::Number>::Cast(minResult)->Value());

    v8::Handle<v8::Function> maxFn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->calculateMaxContentInlineSize().v8Value());
    v8::Handle<v8::Value> maxResult = script->callFunction(maxFn, v8::Null(isolate), 1, argv);
    maxLogicalWidth = LayoutUnit(v8::Handle<v8::Number>::Cast(maxResult)->Value());
}

void LayoutWorker::calculateHeightAndPositionChildren(LayoutUnit& height, Vector<RenderBox*>& children)
{
    ScriptState::Scope scope(m_layoutGlobalScope->calculateHeightAndPositionChildren().scriptState());
    LayoutScriptController* script = m_layoutGlobalScope->script();
    v8::Isolate* isolate = script->isolate();

    v8::Handle<v8::Array> arr = v8::Array::New(isolate, children.size());

    for (size_t i = 0; i < children.size(); ++i) {
        RenderBox* child = children[i];

        // Create a LayoutNode wrapper for each child, if it doesn't exist already.
        if (!child->hasScriptLayoutNode())
            child->setScriptLayoutNode(LayoutNode::create(child));

        // NOTE make this a persistent handle on RenderBoxRareData? How
        // expensive is toV8? I think ScriptWrappable contains m_wrapper which
        // is persistent.
        v8::Handle<v8::Value> node = toV8(child->scriptLayoutNode(), script->context()->Global(), isolate);

        arr->Set(i, node);
    }

    v8::Handle<v8::Value> argv[] = { arr };
    v8::Handle<v8::Function> fn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->calculateHeightAndPositionChildren().v8Value());
    v8::Handle<v8::Value> result = script->callFunction(fn, v8::Null(isolate), 1, argv);
    height = LayoutUnit(v8::Handle<v8::Number>::Cast(result)->Value());
}

const AtomicString& LayoutWorker::interfaceName() const
{
    return EventTargetNames::LayoutWorker;
}

void LayoutWorker::didReceiveResponse(unsigned long identifier, const ResourceResponse&)
{
    WTF_LOG(NotYetImplemented, "didReceiveResponse");
}

void LayoutWorker::notifyFinished()
{
    if (m_scriptLoader->failed()) {
        dispatchEvent(Event::createCancelable(EventTypeNames::error));
    } else {
        WTF_LOG(NotYetImplemented, "%s\n", m_scriptLoader->script().ascii().data());

        // Create global scope & eval script.
        m_layoutGlobalScope = LayoutGlobalScope::create(m_scriptLoader->url(), executionContext()->userAgent(m_scriptLoader->url()));
        m_layoutGlobalScope->script()->evaluate(ScriptSourceCode(m_scriptLoader->script(), m_scriptLoader->url()));
    }
    m_scriptLoader = nullptr;
}


void LayoutWorker::trace(Visitor* visitor)
{
    AbstractWorker::trace(visitor);
}

} // namespace blink
