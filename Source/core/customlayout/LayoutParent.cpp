#include "config.h"
#include "core/customlayout/LayoutParent.h"
#include "core/rendering/RenderBox.h"

namespace blink {

PassRefPtrWillBeRawPtr<LayoutParent> LayoutParent::create(RenderBox* renderBox)
{
    return adoptRefWillBeNoop(new LayoutParent(renderBox));
}

LayoutParent::LayoutParent(RenderBox* renderBox)
    : m_renderBox(renderBox)
{
}

LayoutParent::~LayoutParent()
{
}

double LayoutParent::width() const
{
    // takes into account column width...
    return m_renderBox->availableLogicalWidth();
}

}
