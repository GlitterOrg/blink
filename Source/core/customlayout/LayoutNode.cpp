#include "config.h"
#include "core/customlayout/LayoutNode.h"
#include "core/rendering/RenderBox.h"
#include "platform/geometry/LayoutPoint.h"

namespace blink {

PassRefPtrWillBeRawPtr<LayoutNode> LayoutNode::create(RenderBox* renderBox)
{
    return adoptRefWillBeNoop(new LayoutNode(renderBox));
}

LayoutNode::LayoutNode(RenderBox* renderBox)
  : m_renderBox(renderBox)
{
}

LayoutNode::~LayoutNode()
{
}

double LayoutNode::maxContentInlineSize() const
{
    return m_renderBox->maxPreferredLogicalWidth().toDouble();
}

double LayoutNode::minContentInlineSize() const
{
    return m_renderBox->minPreferredLogicalWidth().toDouble();
}

void LayoutNode::setPosition(double x, double y)
{
    m_renderBox->setLocation(LayoutPoint(x, y));
}

} // namespace blink
