#ifndef LayoutScriptController_h
#define LayoutScriptController_h

#include "bindings/core/v8/ScriptValue.h"
#include "bindings/core/v8/V8Binding.h"
#include "wtf/OwnPtr.h"
#include "wtf/text/TextPosition.h"
#include <v8.h>

namespace blink {

class ErrorEvent;
class LayoutGlobalScope;
class ScriptSourceCode;

class LayoutScriptController {
public:
    explicit LayoutScriptController(LayoutGlobalScope&);
    ~LayoutScriptController();

    bool initializeContextIfNeeded();

    // Returns true if the evaluation completed with no uncaught exception.
    bool evaluate(const ScriptSourceCode&, RefPtrWillBeRawPtr<ErrorEvent>* = 0);

    v8::Local<v8::Value> callFunction(v8::Handle<v8::Function> function, v8::Handle<v8::Value> receiver, int argc, v8::Handle<v8::Value> info[]);

    // Used by V8 bindings:
    v8::Local<v8::Context> context() { return m_scriptState ? m_scriptState->context() : v8::Local<v8::Context>(); }
    v8::Isolate* isolate() { return m_isolate; };

private:
    bool isContextInitialized() { return m_scriptState && !!m_scriptState->perContextData(); }

    ScriptValue evaluate(const String&, const String&, const TextPosition&);

    v8::Isolate* m_isolate;
    LayoutGlobalScope& m_layoutGlobalScope;
    RefPtr<ScriptState> m_scriptState;
    RefPtr<DOMWrapperWorld> m_world;

};

} // namespace blink

#endif // LayoutScriptController_h
