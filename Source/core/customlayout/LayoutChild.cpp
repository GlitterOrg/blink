#include "config.h"
#include "core/customlayout/LayoutChild.h"
#include "core/rendering/RenderBox.h"
#include "platform/geometry/LayoutPoint.h"
#include "core/css/LayoutStyleCSSValueMapping.h"
#include "core/css/HashTools.h"

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

String LayoutChild::getCSSValue(String value) const
{
    const Property* hashTableEntry = findProperty(value.ascii().data(), value.length());
    if (!hashTableEntry)
        return "";
    CSSPropertyID property = static_cast<CSSPropertyID>(hashTableEntry->id);

    return LayoutStyleCSSValueMapping::get(property, m_renderBox->styleRef())->cssText();
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

void LayoutChild::constrainWidth(double width)
{
    //m_renderBox->setOverrideLogicalContentWidth(width);
    m_renderBox->setOverrideContainingBlockContentLogicalWidth(LayoutUnit(width));
}

double LayoutChild::measureWidth()
{
    m_renderBox->setNeedsLayout(MarkOnlyThis);
    m_renderBox->layout();
    return width();
}

double LayoutChild::measureHeight()
{
    m_renderBox->setNeedsLayout(MarkOnlyThis);
    m_renderBox->layout();
    return height();
}

double LayoutChild::width() const
{
    return m_renderBox->size().width();
}

double LayoutChild::height() const
{
    return m_renderBox->size().height();
}

} // namespace blink
