#include "config.h"
#include "bindings/core/v8/V8LayoutGlobalScope.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/ScheduledAction.h"
#include "bindings/core/v8/V8Binding.h"
#include "bindings/core/v8/LayoutScriptController.h"
#include "core/frame/DOMTimer.h"
#include "core/frame/DOMWindowTimers.h"
#include "core/frame/csp/ContentSecurityPolicy.h"
#include "core/inspector/ScriptCallStack.h"
#include "core/workers/LayoutGlobalScope.h"
#include "wtf/OwnPtr.h"

namespace blink {

v8::Handle<v8::Value> toV8(LayoutGlobalScope* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    // Notice that we explicitly ignore creationContext because the LayoutGlobalScope is its own creationContext.

    if (!impl)
        return v8::Null(isolate);

    LayoutScriptController* script = impl->script();
    if (!script)
        return v8::Null(isolate);

    v8::Handle<v8::Object> global = script->context()->Global();
    ASSERT(!global.IsEmpty());
    return global;
}

} // namespace blink
