#include "config.h"

#include "core/customlayout/LayoutChild.h"
#include "core/rendering/RenderBox.h"
#include "core/dom/Document.h"
#include "core/dom/Node.h"
#include "core/layout/style/LayoutStyle.h"
#include "platform/geometry/LayoutPoint.h"
#include "core/css/CSSPrimitiveValueMappings.h"
#include "core/css/CSSToLengthConversionData.h"
#include "core/css/LayoutStyleCSSValueMapping.h"
#include "core/css/HashTools.h"
#include "core/css/StylePropertySet.h"

#include "platform/Logging.h"
#include "platform/TraceEvent.h"

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
    //TRACE_EVENT0("blink", "LayoutChild::getCSSValue");
    const Property* hashTableEntry = findProperty(value.ascii().data(), value.length());
    if (!hashTableEntry)
        return "";
    CSSPropertyID property = static_cast<CSSPropertyID>(hashTableEntry->id);

    return LayoutStyleCSSValueMapping::get(property, m_renderBox->styleRef(), m_renderBox, m_renderBox->node())->cssText();
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
    TRACE_EVENT0("blink", "LayoutChild::constrainWidth");
    m_renderBox->setOverrideLogicalContentWidth(width);
    //m_renderBox->setNeedsLayout(MarkOnlyThis);
    //m_renderBox->layout();
    //m_renderBox->setOverrideContainingBlockContentLogicalWidth(LayoutUnit(width));
}

void LayoutChild::clearOverrideSize()
{
    m_renderBox->clearOverrideSize();
}

double LayoutChild::measureWidth()
{
    m_renderBox->setNeedsLayout(MarkOnlyThis);
    m_renderBox->layout();
    return width();
}

double LayoutChild::measureWidthUsing(String widthStr, double availibleSpace, bool mainSize)
{
    //TRACE_EVENT0("blink", "LayoutChild::measureWidthUsing");
    SizeType sizeType = mainSize ? MainOrPreferredSize : MinSize; // TODO accept MaxSize
    LayoutUnit space(availibleSpace);

    PassRefPtrWillBeRawPtr<MutableStylePropertySet> propertySet = MutableStylePropertySet::create();
    propertySet->setProperty(CSSPropertyWidth, widthStr);
    CSSValue* value = propertySet->getPropertyCSSValue(CSSPropertyWidth).get();
    const LayoutStyle* rootStyle = m_renderBox->document().documentElement()->renderer()->style();
    CSSToLengthConversionData conversionData(m_renderBox->style(), rootStyle, m_renderBox->view(), m_renderBox->style()->effectiveZoom());
    Length size = toCSSPrimitiveValue(value)->convertToLength<AnyConversion>(conversionData);

    return m_renderBox->computeLogicalWidthUsing(sizeType, size, space, m_renderBox->containingBlock()).toDouble();
}

double LayoutChild::measureWidthUsingFixed(double widthPx, double availibleSpace, bool mainSize)
{
    //TRACE_EVENT0("blink", "LayoutChild::measureWidthUsingFixed");
    SizeType sizeType = mainSize ? MainOrPreferredSize : MinSize; // TODO accept MaxSize
    LayoutUnit space(availibleSpace);
    Length size(widthPx, Fixed);

    double result = m_renderBox->computeLogicalWidthUsing(sizeType, size, space, m_renderBox->containingBlock()).toDouble();
    WTF_LOG(NotYetImplemented, "measureWidthUsingFixed: %lf, %lf, %lf, %d", widthPx, availibleSpace, result, mainSize);
    return result;
}

double LayoutChild::measureHeight()
{
    //TRACE_EVENT0("blink", "LayoutChild::measureHeight");
    m_renderBox->setNeedsLayout(MarkOnlyThis);
    m_renderBox->forceChildLayout();
    return height();
}

double LayoutChild::measureHeightAndConstrain(double width)
{
    //TRACE_EVENT0("blink", "LayoutChild::measureHeight");
    m_renderBox->setOverrideLogicalContentWidth(width);
    m_renderBox->setNeedsLayout(MarkOnlyThis);
    m_renderBox->forceChildLayout();
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
