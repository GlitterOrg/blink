#ifndef LayoutContext_h
#define LayoutContext_h

#include "bindings/core/v8/ScriptWrappable.h"

namespace blink {

class Element;
class ExceptionState;

class LayoutContext final : public GarbageCollected<LayoutContext>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO(); // ???
public:
    LayoutContext(Element* element)
        : m_element(element)
    {
    }

    Element* element() const { return m_element; }

    void setLayoutDimensions(float, float);
    void setOffsetForChild(Node* node, float x, float y);

    float availableWidth(Node*, ExceptionState&);
    float availableHeight(Node*, ExceptionState&);

    void constrainWidth(Node*, float width, ExceptionState&);

    void trace(Visitor* visitor);

private:
    // FIXME: Lifecycle.
    Element* m_element;
};

} // namespace blink

#endif // LayoutContext_h
