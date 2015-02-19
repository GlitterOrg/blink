#ifndef Layout_h
#define Layout_h

#include "core/customlayout/LayoutGlobalScope.h"
#include "core/workers/AbstractWorker.h"
#include "core/workers/WorkerScriptLoaderClient.h"
#include "wtf/RefPtr.h"
#include <v8.h>

namespace blink {

class ExceptionState;
class ExecutionContext;
class WorkerScriptLoader;

class Layout final : public AbstractWorker, private WorkerScriptLoaderClient {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtrWillBeRawPtr<Layout> create(ExecutionContext*, const String& url, ExceptionState&);
    virtual ~Layout();

    const AtomicString& type() const;
    double invoke(double, double) const;

    virtual const AtomicString& interfaceName() const override;

    virtual void trace(Visitor*) override;

private:
    explicit Layout(ExecutionContext*);

    // WorkerScriptLoaderClient callbacks
    virtual void didReceiveResponse(unsigned long identifier, const ResourceResponse&) override;
    virtual void notifyFinished() override;

    RefPtr<WorkerScriptLoader> m_scriptLoader;
    RefPtrWillBePersistent<LayoutGlobalScope> m_layoutGlobalScope;
    //v8::Isolate* m_isolate;
};
} // namespace blink

#endif // Layout_h
