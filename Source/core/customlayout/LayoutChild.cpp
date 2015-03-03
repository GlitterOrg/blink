#include "config.h"
#include "core/customlayout/LayoutChild.h"
#include "core/rendering/RenderBox.h"
#include "platform/geometry/LayoutPoint.h"

namespace blink {

PassRefPtrWillBeRawPtr<LayoutChild> LayoutChild::create(RenderBox* renderBox)
{
    return adoptRefWillBeNoop(new LayoutChild(renderBox));
}

LayoutChild::LayoutChild(RenderBox* renderBox)
  : m_renderBox(renderBox)
{
}

LayoutChild::~LayoutChild()
{
}

double LayoutChild::maxContentInlineSize() const
{
    return m_renderBox->maxPreferredLogicalWidth().toDouble();
}

double LayoutChild::minContentInlineSize() const
{
    return m_renderBox->minPreferredLogicalWidth().toDouble();
}

void LayoutChild::setPosition(double x, double y)
{
    m_renderBox->setLocation(LayoutPoint(x, y));
}

} // namespace blink
