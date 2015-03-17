#include "config.h"
#include "core/customlayout/LayoutNode.h"
#include "core/customlayout/LayoutParent.h"
#include "core/customlayout/LayoutChild.h"
#include "core/rendering/RenderBox.h"
#include "core/rendering/RenderBlock.h"
#include "core/css/LayoutStyleCSSValueMapping.h"
#include "core/css/HashTools.h"

#include "platform/Logging.h"

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

LayoutParent* LayoutNode::parent() const
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

double LayoutNode::width() const
{
  return m_renderBox->size().width().toDouble(); // should be inline size. need to add safety here.
}

double LayoutNode::height() const
{
  return m_renderBox->size().height().toDouble(); // should be box size. need to add safety here.
}
void LayoutNode::log(String str) const
{
    WTF_LOG(NotYetImplemented, "log: %s\n", str.ascii().data());
}

}
