#ifndef LayoutWorker_h
#define LayoutWorker_h

#include "core/customlayout/LayoutGlobalScope.h"
#include "core/workers/AbstractWorker.h"
#include "core/workers/WorkerScriptLoaderClient.h"
#include "wtf/RefPtr.h"
#include <v8.h>

namespace blink {

class ExceptionState;
class ExecutionContext;
class LayoutUnit;
class RenderBox;
class RenderCustomBox;
class WorkerScriptLoader;

class LayoutWorker final : public AbstractWorker, private WorkerScriptLoaderClient {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtrWillBeRawPtr<LayoutWorker> create(ExecutionContext*, const String& url, ExceptionState&);
    virtual ~LayoutWorker();

    const AtomicString& type() const;
    double invoke(double, double) const;

    void computeIntrinsicLogicalWidths(LayoutUnit&, LayoutUnit&, RenderCustomBox&);

    void calculateWidth(LayoutUnit&, RenderCustomBox&);
    void calculateHeight(LayoutUnit&, RenderCustomBox&);
    void positionChildren(RenderCustomBox&);

    virtual const AtomicString& interfaceName() const override;

    virtual void trace(Visitor*) override;

private:
    explicit LayoutWorker(ExecutionContext*);

    // WorkerScriptLoaderClient callbacks
    virtual void didReceiveResponse(unsigned long identifier, const ResourceResponse&) override;
    virtual void notifyFinished() override;

    RefPtr<WorkerScriptLoader> m_scriptLoader;
    RefPtrWillBePersistent<LayoutGlobalScope> m_layoutGlobalScope;
};
} // namespace blink

#endif // LayoutWorker_h
