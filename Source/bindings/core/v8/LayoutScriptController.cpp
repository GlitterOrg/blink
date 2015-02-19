#include "config.h"
#include "bindings/core/v8/LayoutScriptController.h"

#include "bindings/core/v8/ScriptSourceCode.h"
#include "bindings/core/v8/ScriptValue.h"
#include "bindings/core/v8/V8DOMWrapper.h"
#include "bindings/core/v8/V8ObjectConstructor.h"
#include "bindings/core/v8/V8ScriptRunner.h"
#include "bindings/core/v8/WrapperTypeInfo.h"
#include "core/customlayout/LayoutGlobalScope.h"
#include "core/events/ErrorEvent.h"
#include "core/frame/DOMTimer.h"
#include "core/inspector/ScriptCallStack.h"
#include "public/platform/Platform.h"
#include "platform/Logging.h"
#include <v8.h>

namespace blink {

LayoutScriptController::LayoutScriptController(LayoutGlobalScope& layoutGlobalScope)
    : m_isolate(0)
    , m_layoutGlobalScope(layoutGlobalScope)
{
    m_isolate = v8::Isolate::GetCurrent();
    m_world = DOMWrapperWorld::create(m_isolate, LayoutWorldId);
}

LayoutScriptController::~LayoutScriptController()
{
    m_world->dispose();
}

bool LayoutScriptController::initializeContextIfNeeded()
{
    v8::HandleScope handleScope(m_isolate);

    if (isContextInitialized())
        return true;

    v8::Handle<v8::Context> context = v8::Context::New(m_isolate);
    if (context.IsEmpty())
        return false;

    m_scriptState = ScriptState::create(context, m_world);

    ScriptState::Scope scope(m_scriptState.get());

    // Name new context for debugging. XXX may break
    V8PerContextDebugData::setContextDebugData(context, "layout");

    // Create a new JS object and use it as the prototype for the shadow global object. XXX may break
    const WrapperTypeInfo* wrapperTypeInfo = m_layoutGlobalScope.wrapperTypeInfo();
    v8::Handle<v8::Function> layoutGlobalScopeConstructor = m_scriptState->perContextData()->constructorForType(wrapperTypeInfo);
    v8::Local<v8::Object> jsLayoutGlobalScope = V8ObjectConstructor::newInstance(m_isolate, layoutGlobalScopeConstructor);
    if (jsLayoutGlobalScope.IsEmpty()) {
        m_scriptState->disposePerContextData();
        return false;
    }

    V8DOMWrapper::associateObjectWithWrapper(m_isolate, &m_layoutGlobalScope, wrapperTypeInfo, jsLayoutGlobalScope);

    v8::Handle<v8::Object> globalObject = v8::Handle<v8::Object>::Cast(m_scriptState->context()->Global()->GetPrototype());
    globalObject->SetPrototype(jsLayoutGlobalScope);

    return true;
}

ScriptValue LayoutScriptController::evaluate(const String& script, const String& fileName, const TextPosition& scriptStartPosition)
{
    if (!initializeContextIfNeeded())
        return ScriptValue();

    ScriptState::Scope scope(m_scriptState.get());

    v8::TryCatch block;

    v8::Handle<v8::String> scriptString = v8String(m_isolate, script);
    v8::Handle<v8::Script> compiledScript = V8ScriptRunner::compileScript(scriptString, fileName, scriptStartPosition, 0, 0, m_isolate);
    v8::Local<v8::Value> result = V8ScriptRunner::runCompiledScript(m_isolate, compiledScript, &m_layoutGlobalScope);

    if (!block.CanContinue()) {
        //m_layoutGlobalScope.script()->forbidExecution(); XXX may break.
        return ScriptValue();
    }

    if (block.HasCaught()) {
        v8::Local<v8::Message> message = block.Message();
        WTF_LOG(NotYetImplemented, "Caught error. %s\n", toCoreString(message->Get()).ascii().data());
    }

    if (result.IsEmpty() || result->IsUndefined())
        return ScriptValue();

    return ScriptValue(m_scriptState.get(), result);
}

bool LayoutScriptController::evaluate(const ScriptSourceCode& sourceCode, RefPtrWillBeRawPtr<ErrorEvent>* errorEvent)
{
    // XXX removed a whole bunch of error reporting here for time.
    evaluate(sourceCode.source(), sourceCode.url().string(), sourceCode.startPosition());
    return true;
}

v8::Local<v8::Value> LayoutScriptController::callFunction(v8::Handle<v8::Function> function, v8::Handle<v8::Value> receiver, int argc, v8::Handle<v8::Value> info[])
{
    return V8ScriptRunner::callFunction(function, m_layoutGlobalScope.executionContext(), receiver, argc, info, m_isolate);
}

} // namespace blink
