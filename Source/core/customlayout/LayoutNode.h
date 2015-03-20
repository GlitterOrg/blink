#ifndef LayoutNode_h
#define LayoutNode_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "wtf/RefPtr.h"

namespace blink {

class LayoutChild;
class LayoutParent;
class RenderBox;

class LayoutNode : public RefCountedWillBeGarbageCollectedFinalized<LayoutNode>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtrWillBeRawPtr<LayoutNode> create(RenderBox*);
    virtual ~LayoutNode();

    LayoutParent* parent() const;
    WillBeHeapVector<LayoutChild*> children() const;

    String getCSSValue(String value) const;
    double width() const;
    double overrideWidth() const;
    double height() const;

    void setWidth(double);
    void setHeight(double);

    void log(String) const;

private:
    LayoutNode(RenderBox*);

    RenderBox* m_renderBox;
};

} // namespace blink

#endif // LayoutNode_h
