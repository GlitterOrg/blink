// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/customlayout/CustomLayoutGlobalScope.h"

// FIXME: Cargo cult #include
#include "bindings/core/v8/ScriptSourceCode.h"
#include "bindings/core/v8/ScriptValue.h"
#include "bindings/core/v8/V8DedicatedWorkerGlobalScope.h"
#include "bindings/core/v8/V8ErrorHandler.h"
#include "bindings/core/v8/V8GCController.h"
#include "bindings/core/v8/V8Initializer.h"
#include "bindings/core/v8/V8ObjectConstructor.h"
#include "bindings/core/v8/V8ScriptRunner.h"
#include "bindings/core/v8/V8SharedWorkerGlobalScope.h"
#include "bindings/core/v8/V8WorkerGlobalScope.h"
#include "bindings/core/v8/WrapperTypeInfo.h"
#include "core/events/ErrorEvent.h"
#include "core/frame/DOMTimer.h"
#include "core/inspector/ScriptCallStack.h"
#include "core/workers/SharedWorkerGlobalScope.h"
#include "core/workers/WorkerGlobalScope.h"
#include "core/workers/WorkerObjectProxy.h"
#include "core/workers/WorkerThread.h"
#include "platform/heap/ThreadState.h"
#include "public/platform/Platform.h"
#include <v8.h>

#include "core/dom/MainThreadTaskRunner.h"
#include "core/FetchInitiatorTypeNames.h"

namespace blink {

PassRefPtrWillBeRawPtr<CustomLayoutGlobalScope> CustomLayoutGlobalScope::create(ExecutionContext* context)
{
    RefPtrWillBeRawPtr<CustomLayoutGlobalScope> globalScope = adoptRefWillBeNoop(new CustomLayoutGlobalScope(context));
    // FIXME: Needed?
    //context->applyContentSecurityPolicyFromString(startupData->m_contentSecurityPolicy, startupData->m_contentSecurityPolicyType);

    globalScope->suspendIfNeeded();

    return globalScope.release();
}

CustomLayoutGlobalScope::CustomLayoutGlobalScope(ExecutionContext* context)
    : ActiveDOMObject(context)
    , m_taskRunner(0) //MainThreadTaskRunner::create(this))
    , m_isolate(0)
    , m_isLoading(false)
    , m_decoder(0)
{
}

CustomLayoutGlobalScope::~CustomLayoutGlobalScope()
{
    //if (isContextInitialized())
    // FIXME: We'are leaking the isolate.
}

const AtomicString& CustomLayoutGlobalScope::interfaceName() const
{
    DEFINE_STATIC_LOCAL(AtomicString, name, ("CustomLayoutGlobalScope"));
    return name;
}

void CustomLayoutGlobalScope::postTask(PassOwnPtr<ExecutionContextTask> task)
{
    m_taskRunner->postTask(task);
}

static void TestNativeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    CustomLayoutGlobalScope* globalScope = (CustomLayoutGlobalScope*)isolate->GetData(0);
    args.GetReturnValue().Set(v8::Number::New(isolate, globalScope->test()));
}

void CustomLayoutGlobalScope::initializeContextIfNeeded()
{
    if (isContextInitialized())
        return;

    v8::Isolate* current = v8::Isolate::GetCurrent();

    {
        m_isolate = v8::Isolate::New();
        m_isolate->SetData(0, this);
        v8::Isolate::Scope isolate_scope(m_isolate);



        v8::HandleScope handle_scope(m_isolate);

        v8::Local<v8::ObjectTemplate> gbl = v8::ObjectTemplate::New();
        gbl->Set(v8::String::NewFromUtf8(m_isolate, "test"), v8::FunctionTemplate::New(m_isolate, TestNativeCallback));
        v8::Handle<v8::Context> context = v8::Context::New(m_isolate, NULL, gbl);
        // Enter the context for compiling and running the script.
        v8::Context::Scope context_scope(context);
        // Create a string containing the JavaScript source code.
        String sourceCode = m_script.toString();
        char* sourceCodeChar = (char*)calloc(sourceCode.length() + 1, sizeof(char*));
        memcpy(sourceCodeChar, sourceCode.characters8(), sourceCode.length() * sizeof(char*));
        sourceCodeChar[sourceCode.length()] = '\0';
        v8::Local<v8::String> source = v8::String::NewFromUtf8(m_isolate, (const char*)sourceCodeChar);
        // Compile the source code.
        v8::Local<v8::Script> script = v8::Script::Compile(source);

        // Run the JS code.
        script->Run();

    }

    ASSERT(current == v8::Isolate::GetCurrent());
}


void CustomLayoutGlobalScope::loadAndRun(ExecutionContext* context, const KURL& scriptURL)
{
    /*
    WorkerScriptController* script = m_workerGlobalScope->script();
    if (!script->isExecutionForbidden())
        script->initializeContextIfNeeded();
    InspectorInstrumentation::willEvaluateWorkerScript(workerGlobalScope(), startMode);
    bool success = script->evaluate(ScriptSourceCode(sourceCode, scriptURL));
    m_workerGlobalScope->didEvaluateWorkerScript();
    m_workerReportingProxy.didEvaluateWorkerScript(success);
    */

    // FIXME: We are resetting m_scriptLoader over and over.
    m_scriptLoader = WorkerScriptLoader::create();
    // FIXME: The scriptLoader gets cancelled despite us being an activeDOMObject :(
    // So that's why we don't do an async load but we SHOULD!
    //m_scriptLoader->loadAsynchronously(*context, scriptURL, DenyCrossOriginRequests, this);
    ResourceRequest request(scriptURL);

    ThreadableLoaderOptions options;
    //options.crossOriginRequestPolicy = UseAccessControl;
    options.initiator = FetchInitiatorTypeNames::document;
    options.contentSecurityPolicyEnforcement = DoNotEnforceContentSecurityPolicy; // FIXME

    ResourceLoaderOptions resourceLoaderOptions;
    resourceLoaderOptions.allowCredentials = DoNotAllowStoredCredentials;
    resourceLoaderOptions.credentialsRequested = ClientDidNotRequestCredentials;
    //resourceLoaderOptions.securityOrigin = securityOrigin();
    //resourceLoaderOptions.mixedContentBlockingTreatment = TreatAsActiveContent;

    ThreadableLoader::loadResourceSynchronously(*context, request, *this, options, resourceLoaderOptions);
    //m_scriptLoader->loadSynchronously(*context, scriptURL, DenyCrossOriginRequests);
    m_isLoading = true;
}

void CustomLayoutGlobalScope::run()
{
    m_isLoading = false;

    initializeContextIfNeeded();
    // evaluate(const String& script, const String& fileName, const TextPosition& scriptStartPosition)
    /*if (!initializeContextIfNeeded())
        return ScriptValue();

    ScriptState::Scope scope(m_scriptState.get());

    m_scriptState->context()->AllowCodeGenerationFromStrings(false);
    m_scriptState->context()->SetErrorMessageForCodeGenerationFromStrings(v8String(m_isolate, "Eval disallowed."));

    v8::TryCatch block;

    v8::Handle<v8::String> scriptString = v8String(m_isolate, script);
    v8::Handle<v8::Script> compiledScript = V8ScriptRunner::compileScript(scriptString, fileName, scriptStartPosition, 0, 0, m_isolate);
    v8::Local<v8::Value> result = V8ScriptRunner::runCompiledScript(m_isolate, compiledScript, &m_workerGlobalScope);

    if (block.HasCaught()) {
        v8::Local<v8::Message> message = block.Message();
        m_globalScopeExecutionState->hadException = true;
        m_globalScopeExecutionState->errorMessage = toCoreString(message->Get());
        m_globalScopeExecutionState->lineNumber = message->GetLineNumber();
        m_globalScopeExecutionState->columnNumber = message->GetStartColumn() + 1;
        TOSTRING_DEFAULT(V8StringResource<>, sourceURL, message->GetScriptOrigin().ResourceName(), ScriptValue());
        m_globalScopeExecutionState->sourceURL = sourceURL;
        m_globalScopeExecutionState->exception = ScriptValue(m_scriptState.get(), block.Exception());
        block.Reset();
    } else {
        m_globalScopeExecutionState->hadException = false;
    }

    if (result.IsEmpty() || result->IsUndefined())
        return ScriptValue();

    return ScriptValue(m_scriptState.get(), result);*/
}

bool CustomLayoutGlobalScope::hasPendingActivity() const
{
    return m_isLoading;
}

void CustomLayoutGlobalScope::didReceiveData(const char* data, unsigned dataLength)
{
    if (!m_decoder) {
        /*if (!m_responseEncoding.isEmpty())
            m_decoder = TextResourceDecoder::create("text/javascript", m_responseEncoding);
        else*/
            m_decoder = TextResourceDecoder::create("text/javascript", "UTF-8");
    }

    if (!dataLength)
        return;

    m_script.append(m_decoder->decode(data, dataLength));
}

void CustomLayoutGlobalScope::didFinishLoading(unsigned long /*identifier*/, double /*finishTime*/)
{
    run();
}

void CustomLayoutGlobalScope::trace(Visitor* visitor)
{
    ActiveDOMObject::trace(visitor);
    ExecutionContext::trace(visitor);

    visitor->trace(m_scriptLoader);
    visitor->trace(m_taskRunner);
}

} // namespace blink
