#include "config.h"
#include "core/customlayout/LayoutWorker.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/V8PerIsolateData.h"
#include "bindings/core/v8/ScriptSourceCode.h"
#include "bindings/core/v8/ScriptState.h"
#include "bindings/core/v8/ToV8.h"
#include "core/events/MessageEvent.h"
#include "core/customlayout/LayoutArgs.h"
#include "core/customlayout/LayoutChild.h"
#include "core/css/LayoutStyleCSSValueMapping.h"
#include "core/rendering/RenderBox.h"
#include "core/rendering/RenderCustomBox.h"
#include "core/workers/WorkerScriptLoader.h"
#include "platform/LayoutUnit.h"
#include "platform/Logging.h"
#include "platform/TraceEvent.h"
#include "wtf/StdLibExtras.h"
#include <v8.h>

namespace blink {

inline LayoutWorker::LayoutWorker(ExecutionContext* context)
    : AbstractWorker(context)
    , m_dirtyProperties(true)
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
    v8::TryCatch tryCatch(isolate);
    LayoutScriptController* script = m_layoutGlobalScope->script();

    v8::Handle<v8::Value> argsHandle = toV8(args.get(), script->context()->Global(), isolate);
    v8::Handle<v8::Value> argv[] = { argsHandle };
    v8::Handle<v8::Function> fn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->fn().v8Value());
    v8::Handle<v8::Value> result = script->callFunction(fn, v8::Null(isolate), 1, argv);

    if (tryCatch.HasCaught()) {
#ifndef NDEBUG
      v8::String::Utf8Value exception_str(tryCatch.Exception());
      const char* cstr = *exception_str;
      WTF_LOG(NotYetImplemented, "v8 exception: %s\n", cstr);
#endif
      return 0; // TODO fail should revert to a sensible renderer.
    }

    return v8::Handle<v8::Number>::Cast(result)->Value();
}

void LayoutWorker::doLayout(RenderCustomBox& renderer)
{
    //TRACE_EVENT0("blink", "LayoutWorker::doLayout");

    ScriptState::Scope scope(m_layoutGlobalScope->doLayout().scriptState());
    LayoutScriptController* script = m_layoutGlobalScope->script();
    v8::Isolate* isolate = script->isolate();
    v8::TryCatch tryCatch(isolate);

    if (m_dirtyProperties) {
        TRACE_EVENT_BEGIN0("blink", "LayoutWorker::doLayout_init");
        WTF_LOG(NotYetImplemented, "LayoutWorker::doLayout_init");

        // Create a list of dictionaries for all the values we might need.
        v8::Handle<v8::Array> properties = v8::Array::New(isolate);

        /*v8::Handle<v8::String> flexGrow = v8::String::NewFromUtf8(isolate, "flex-grow");
        v8::Handle<v8::String> flexShrink = v8::String::NewFromUtf8(isolate, "flex-shrink");
        v8::Handle<v8::String> flexBasis = v8::String::NewFromUtf8(isolate, "flex-basis");
        v8::Handle<v8::String> w = v8::String::NewFromUtf8(isolate, "width");
        v8::Handle<v8::String> alignSelf = v8::String::NewFromUtf8(isolate, "align-self");
        v8::Handle<v8::String> maxWidth = v8::String::NewFromUtf8(isolate, "max-width");
        v8::Handle<v8::String> minWidth = v8::String::NewFromUtf8(isolate, "min-width");*/
        int i = 0;

        for (RenderBox* child = renderer.firstChildBox(); child; child = child->nextSiblingBox()) {
            v8::Handle<v8::Array> dict = v8::Array::New(isolate, 7);
            dict->Set(0, v8::Number::New(isolate, child->style()->flexGrow()));
            dict->Set(1, v8::Number::New(isolate, child->style()->flexShrink()));
            dict->Set(2, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyFlexBasis, child->styleRef(), child, child->node())->cssText().ascii().data()));
            dict->Set(3, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyWidth, child->styleRef(), child, child->node())->cssText().ascii().data()));
            dict->Set(4, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyAlignSelf, child->styleRef(), child, child->node())->cssText().ascii().data()));
            dict->Set(5, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyMaxWidth, child->styleRef(), child, child->node())->cssText().ascii().data()));
            dict->Set(6, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyMinWidth, child->styleRef(), child, child->node())->cssText().ascii().data()));

            properties->Set(i++, dict);
        }

        script->context()->Global()->Set(v8::String::NewFromUtf8(isolate, "props"), properties);
        script->context()->Global()->Set(v8::String::NewFromUtf8(isolate, "cssJustifyContent"),
                                         v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyJustifyContent, renderer.styleRef(), &renderer, renderer.node())->cssText().ascii().data()));

        m_dirtyProperties = false;

        TRACE_EVENT_END0("blink", "LayoutWorker::doLayout_init");
    }

    if (!renderer.hasScriptLayoutNode())
        renderer.setScriptLayoutNode(LayoutNode::create(&renderer));
    v8::Handle<v8::Value> node = toV8(renderer.scriptLayoutNode(), script->context()->Global(), isolate);

    v8::Handle<v8::Value> argv[] = { node };
    v8::Handle<v8::Function> fn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->doLayout().v8Value());
    script->callFunction(fn, v8::Null(isolate), 1, argv);

    if (tryCatch.HasCaught()) {
#ifndef NDEBUG
        v8::String::Utf8Value exception_str(tryCatch.Exception());
        const char* cstr = *exception_str;
        WTF_LOG(NotYetImplemented, "v8 exception: %s\n", cstr);
#endif
    }
}

void LayoutWorker::computeIntrinsicLogicalWidths(LayoutUnit& minLogicalWidth, LayoutUnit& maxLogicalWidth, RenderCustomBox& renderer)
{
    ScriptState::Scope scope(m_layoutGlobalScope->calculateMinContentInlineSize().scriptState());
    LayoutScriptController* script = m_layoutGlobalScope->script();
    v8::Isolate* isolate = script->isolate();
    v8::TryCatch tryCatch(isolate);

    if (!renderer.hasScriptLayoutNode())
        renderer.setScriptLayoutNode(LayoutNode::create(&renderer));
    v8::Handle<v8::Value> node = toV8(renderer.scriptLayoutNode(), script->context()->Global(), isolate);

    v8::Handle<v8::Value> argv[] = { node };
    v8::Handle<v8::Function> minFn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->calculateMinContentInlineSize().v8Value());
    v8::Handle<v8::Value> minResult = script->callFunction(minFn, v8::Null(isolate), 1, argv);

    if (tryCatch.HasCaught()) {
#ifndef NDEBUG
        v8::String::Utf8Value exception_str(tryCatch.Exception());
        const char* cstr = *exception_str;
        WTF_LOG(NotYetImplemented, "v8 exception: %s\n", cstr);
#endif
    } else {
        minLogicalWidth = LayoutUnit(v8::Handle<v8::Number>::Cast(minResult)->Value());
    }

    v8::Handle<v8::Function> maxFn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->calculateMaxContentInlineSize().v8Value());
    v8::Handle<v8::Value> maxResult = script->callFunction(maxFn, v8::Null(isolate), 1, argv);

    if (tryCatch.HasCaught()) {
#ifndef NDEBUG
        v8::String::Utf8Value exception_str(tryCatch.Exception());
        const char* cstr = *exception_str;
        WTF_LOG(NotYetImplemented, "v8 exception: %s\n", cstr);
#endif
    } else {
        maxLogicalWidth = LayoutUnit(v8::Handle<v8::Number>::Cast(maxResult)->Value());
    }
}

void LayoutWorker::calculateWidth(LayoutUnit& width, RenderCustomBox& renderer)
{
    ScriptState::Scope scope(m_layoutGlobalScope->calculateWidth().scriptState());
    LayoutScriptController* script = m_layoutGlobalScope->script();
    v8::Isolate* isolate = script->isolate();
    v8::TryCatch tryCatch(isolate);

    TRACE_EVENT_BEGIN0("blink", "LayoutWorker::calculateWidth");

    // Create a list of dictionaries for all the values we might need.
    v8::Handle<v8::Array> properties = v8::Array::New(isolate);

    v8::Handle<v8::String> flexGrow = v8::String::NewFromUtf8(isolate, "flex-grow");
    v8::Handle<v8::String> flexShrink = v8::String::NewFromUtf8(isolate, "flex-shrink");
    v8::Handle<v8::String> flexBasis = v8::String::NewFromUtf8(isolate, "flex-basis");
    v8::Handle<v8::String> w = v8::String::NewFromUtf8(isolate, "width");
    v8::Handle<v8::String> alignSelf = v8::String::NewFromUtf8(isolate, "align-self");
    v8::Handle<v8::String> maxWidth = v8::String::NewFromUtf8(isolate, "max-width");
    v8::Handle<v8::String> minWidth = v8::String::NewFromUtf8(isolate, "min-width");
    int i = 0;

    for (RenderBox* child = renderer.firstChildBox(); child; child = child->nextSiblingBox()) {
        v8::Handle<v8::Object> dict = v8::Object::New(isolate);
        dict->Set(flexGrow, v8::Number::New(isolate, child->style()->flexGrow()));
        dict->Set(flexShrink, v8::Number::New(isolate, child->style()->flexShrink()));
        dict->Set(flexBasis, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyFlexBasis, child->styleRef(), child, child->node())->cssText().ascii().data()));
        dict->Set(w, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyWidth, child->styleRef(), child, child->node())->cssText().ascii().data()));
        dict->Set(alignSelf, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyAlignSelf, child->styleRef(), child, child->node())->cssText().ascii().data()));
        dict->Set(maxWidth, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyMaxWidth, child->styleRef(), child, child->node())->cssText().ascii().data()));
        dict->Set(minWidth, v8::String::NewFromUtf8(isolate, LayoutStyleCSSValueMapping::get(CSSPropertyMinWidth, child->styleRef(), child, child->node())->cssText().ascii().data()));

        properties->Set(i++, dict);
    }

    script->context()->Global()->Set(v8::String::NewFromUtf8(isolate, "props"), properties);

    TRACE_EVENT_END0("blink", "LayoutWorker::calculateWidth");

    if (!renderer.hasScriptLayoutNode())
        renderer.setScriptLayoutNode(LayoutNode::create(&renderer));
    v8::Handle<v8::Value> node = toV8(renderer.scriptLayoutNode(), script->context()->Global(), isolate);

    v8::Handle<v8::Value> argv[] = { node };
    v8::Handle<v8::Function> fn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->calculateWidth().v8Value());
    v8::Handle<v8::Value> result = script->callFunction(fn, v8::Null(isolate), 1, argv);

    if (tryCatch.HasCaught()) {
#ifndef NDEBUG
        v8::String::Utf8Value exception_str(tryCatch.Exception());
        const char* cstr = *exception_str;
        WTF_LOG(NotYetImplemented, "v8 exception: %s\n", cstr);
#endif
    } else {
        width = LayoutUnit(v8::Handle<v8::Number>::Cast(result)->Value());
    }
}

void LayoutWorker::calculateHeight(LayoutUnit& height, RenderCustomBox& renderer)
{
    ScriptState::Scope scope(m_layoutGlobalScope->calculateHeight().scriptState());
    LayoutScriptController* script = m_layoutGlobalScope->script();
    v8::Isolate* isolate = script->isolate();
    v8::TryCatch tryCatch(isolate);

    if (!renderer.hasScriptLayoutNode())
        renderer.setScriptLayoutNode(LayoutNode::create(&renderer));
    v8::Handle<v8::Value> node = toV8(renderer.scriptLayoutNode(), script->context()->Global(), isolate);

    v8::Handle<v8::Value> argv[] = { node };
    v8::Handle<v8::Function> fn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->calculateHeight().v8Value());
    v8::Handle<v8::Value> result = script->callFunction(fn, v8::Null(isolate), 1, argv);

    if (tryCatch.HasCaught()) {
#ifndef NDEBUG
        v8::String::Utf8Value exception_str(tryCatch.Exception());
        const char* cstr = *exception_str;
        WTF_LOG(NotYetImplemented, "v8 exception: %s\n", cstr);
#endif
    } else {
        height = LayoutUnit(v8::Handle<v8::Number>::Cast(result)->Value());
    }
}

void LayoutWorker::positionChildren(RenderCustomBox& renderer)
{
    ScriptState::Scope scope(m_layoutGlobalScope->positionChildren().scriptState());
    LayoutScriptController* script = m_layoutGlobalScope->script();
    v8::Isolate* isolate = script->isolate();
    v8::TryCatch tryCatch(isolate);

    if (!renderer.hasScriptLayoutNode())
        renderer.setScriptLayoutNode(LayoutNode::create(&renderer));
    v8::Handle<v8::Value> node = toV8(renderer.scriptLayoutNode(), script->context()->Global(), isolate);

    v8::Handle<v8::Value> argv[] = { node };
    v8::Handle<v8::Function> fn = v8::Handle<v8::Function>::Cast(m_layoutGlobalScope->positionChildren().v8Value());
    script->callFunction(fn, v8::Null(isolate), 1, argv);

#ifndef NDEBUG
    if (tryCatch.HasCaught()) {
        v8::String::Utf8Value exception_str(tryCatch.Exception());
        const char* cstr = *exception_str;
        WTF_LOG(NotYetImplemented, "v8 exception: %s\n", cstr);
    }
#endif
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
