#include "config.h"
#include "core/dom/LayoutContext.h"

#include "core/dom/Element.h"
#include "core/rendering/RenderBox.h"
#include "bindings/core/v8/ExceptionState.h"

namespace blink {

void LayoutContext::setLayoutDimensions(float width, float height)
{
    m_element->renderBox()->setWidth(width);
    m_element->renderBox()->setHeight(height);
}

void LayoutContext::setOffsetForChild(Node* node, float x, float y)
{
    RenderBox* renderer = node->renderBox();
    if (!renderer)
        return;

    renderer->setX(x);
    renderer->setY(y);
}

float LayoutContext::availableWidth(Node* node, ExceptionState& state)
{
    if (!node) {
        state.throwDOMException(NotSupportedError, "|node| is null.");
        return 0;
    }

    RenderBox* box = node->renderBox();
    if (!box)
        return 0;

    return box->availableLogicalWidth().toFloat();
}

float LayoutContext::availableHeight(Node* node, ExceptionState& state)
{
    if (!node) {
        state.throwDOMException(NotSupportedError, "|node| is null.");
        return 0;
    }

    RenderBox* box = node->renderBox();
    if (!box)
        return 0;

    return box->availableLogicalHeight(ExcludeMarginBorderPadding).toFloat();
}

void LayoutContext::constrainWidth(Node* node, float width, ExceptionState& state)
{
    RenderBox* box = node->renderBox();
    if (!box) {
        state.throwDOMException(NotSupportedError, "NOt called on a box.");
        return;
    }

    box->setOverrideLogicalContentWidth(width);
}

void LayoutContext::trace(Visitor* visitor)
{
    visitor->trace(*m_element);
}

} // namespace blink
