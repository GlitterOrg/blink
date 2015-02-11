// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CustomLayoutGlobalScope_h
#define CustomLayoutGlobalScope_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/V8Binding.h"
#include "core/dom/ExecutionContext.h"
#include "core/workers/WorkerScriptLoader.h"
#include "core/workers/WorkerScriptLoaderClient.h"
#include "core/loader/ThreadableLoaderClient.h"
#include "wtf/text/StringBuilder.h"
#include <v8.h>

#include "core/dom/ActiveDOMObject.h"
#include "core/html/parser/TextResourceDecoder.h"

namespace blink {

class MainThreadTaskRunner;

// FIXME: Do we really need those 15,000 inheritance (esp. SecurityContext and ScriptWrappable).
class CustomLayoutGlobalScope final : public ExecutionContext, public ScriptWrappable, public SecurityContext, public WorkerScriptLoaderClient, public ActiveDOMObject, public ThreadableLoaderClient {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtrWillBeRawPtr<CustomLayoutGlobalScope> create(ExecutionContext*);
    virtual const AtomicString& interfaceName() const;

    void loadAndRun(ExecutionContext*, const KURL&);

    double test() const { return 1; }

    // ExecutionContext
    virtual void disableEval(const String& errorMessage) override { }
    virtual String userAgent(const KURL&) const override { return String("User-Agent"); }
    virtual void postTask(PassOwnPtr<ExecutionContextTask>) override;
    virtual double timerAlignmentInterval() const override { return 150.0; }
    virtual DOMTimerCoordinator* timers() override { return 0; }
    virtual void reportBlockedScriptExecutionToInspector(const String& directiveText) override { }
    virtual SecurityContext& securityContext() override { return *this; }
    virtual void addConsoleMessage(PassRefPtrWillBeRawPtr<ConsoleMessage>) override { }
    virtual void logExceptionToConsole(const String& errorMessage, int scriptId, const String& sourceURL, int lineNumber, int columnNumber, PassRefPtrWillBeRawPtr<ScriptCallStack>) override { }
    virtual EventTarget* errorEventTarget() override { return nullptr; }
    virtual EventQueue* eventQueue() const override { return nullptr; }
    virtual const KURL& virtualURL() const override { return m_url; }
    virtual KURL virtualCompleteURL(const String&) const override { return KURL(); }

#if !ENABLE(OILPAN)
    virtual void refExecutionContext() override {
        // FIXME: Bad.
    }
    virtual void derefExecutionContext() override {
        // FIXME: Bad.
    }
#endif

    // SecurityContext
    virtual void didUpdateSecurityOrigin() override { }

    // ActiveDOMObject
    virtual bool hasPendingActivity() const override;

    // ThreadableLoaderClient
    virtual void didReceiveData(const char*, unsigned /*dataLength*/) override;
    virtual void didFinishLoading(unsigned long /*identifier*/, double /*finishTime*/) override;

    virtual void trace(Visitor*);

    ~CustomLayoutGlobalScope();

private:
    CustomLayoutGlobalScope(ExecutionContext*);

    bool isContextInitialized() { return m_isolate; }

    void initializeContextIfNeeded();

    virtual void notifyFinished() override { run(); }

    void run();

    RefPtr<WorkerScriptLoader> m_scriptLoader;

    KURL m_url;

    OwnPtr<MainThreadTaskRunner> m_taskRunner;
    v8::Isolate* m_isolate;

    bool m_isLoading;

    // For sync loading
    OwnPtr<TextResourceDecoder> m_decoder;
    StringBuilder m_script;
};

} // namespace blink

#endif // CustomLayoutGlobalScope_h
