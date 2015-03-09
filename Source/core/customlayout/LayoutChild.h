#ifndef LayoutChild_h
#define LayoutChild_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "wtf/RefPtr.h"

namespace blink {

class RenderBox;

class LayoutChild : public RefCountedWillBeGarbageCollectedFinalized<LayoutChild>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtrWillBeRawPtr<LayoutChild> create(RenderBox*);
    virtual ~LayoutChild();

    String getCSSValue(String value) const;

    double maxContentInlineSize() const;
    double minContentInlineSize() const;
    void setPosition(double x, double y);

    void constrainWidth(double width);
    double measureWidth();
    double measureHeight();

    double width() const;
    double height() const;

private:
    LayoutChild(RenderBox*);

    RenderBox* m_renderBox;
};

} // namespace blink

#endif // LayoutChild_h
