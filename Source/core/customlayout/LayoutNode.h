#ifndef LayoutNode_h
#define LayoutNode_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "wtf/RefPtr.h"

namespace blink {

class RenderBox;

class LayoutNode : public RefCountedWillBeGarbageCollectedFinalized<LayoutNode>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtrWillBeRawPtr<LayoutNode> create(RenderBox*);
    virtual ~LayoutNode();
    double maxContentInlineSize() const;
    double minContentInlineSize() const;
    void setPosition(double x, double y);

private:
    LayoutNode(RenderBox*);

    RenderBox* m_renderBox;
};

} // namespace blink

#endif // LayoutNode_h
