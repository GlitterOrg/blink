#ifndef LayoutParent_h
#define LayoutParent_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "wtf/RefPtr.h"

namespace blink {

class RenderBox;

class LayoutParent : public RefCountedWillBeGarbageCollectedFinalized<LayoutParent>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtrWillBeRawPtr<LayoutParent> create(RenderBox*);
    virtual ~LayoutParent();
    double width() const;

private:
    LayoutParent(RenderBox*);

    RenderBox* m_renderBox;
};

} // namespace blink

#endif // LayoutParent_h
