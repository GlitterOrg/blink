#include "config.h"
#include "core/customlayout/LayoutNode.h"
#include "core/customlayout/LayoutParent.h"
#include "core/customlayout/LayoutChild.h"
#include "core/rendering/RenderBox.h"
#include "core/rendering/RenderBlock.h"
#include "core/css/LayoutStyleCSSValueMapping.h"
#include "core/css/HashTools.h"

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

RefPtr<LayoutParent> LayoutNode::parent() const
{
    RenderBlock* parent = m_renderBox->containingBlock();
    if (!parent->hasScriptLayoutParent())
        parent->setScriptLayoutParent(LayoutParent::create(parent));
    return parent->scriptLayoutParent();
}

WillBeHeapVector<LayoutChild*> LayoutNode::children() const
{
    WillBeHeapVector<LayoutChild*> children;

    for (RenderBox* child = m_renderBox->firstChildBox(); child; child = child->nextSiblingBox()) {
        if (!child->hasScriptLayoutChild())
            child->setScriptLayoutChild(LayoutChild::create(child));
        children.append(child->scriptLayoutChild());
    }

    return children;
}

String LayoutNode::getCSSValue(String value) const
{
    const Property* hashTableEntry = findProperty(value.ascii().data(), value.length());
    if (!hashTableEntry)
        return "";
    CSSPropertyID property = static_cast<CSSPropertyID>(hashTableEntry->id);

    return LayoutStyleCSSValueMapping::get(property, m_renderBox->styleRef())->cssText();
}

}
