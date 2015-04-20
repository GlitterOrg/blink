#include "config.h"
#include "bindings/core/v8/ScriptPromise.h"
#include "bindings/core/v8/ScriptState.h"
#include "bindings/core/v8/ScriptFunction.h"
#include "core/customlayout/LayoutNode.h"
#include "core/customlayout/LayoutParent.h"
#include "core/customlayout/LayoutChild.h"
#include "core/rendering/RenderBox.h"
#include "core/rendering/RenderBlock.h"
#include "core/css/LayoutStyleCSSValueMapping.h"
#include "core/css/HashTools.h"

#include "platform/Logging.h"

namespace blink {

class LayoutNode::ThenFunction final : public ScriptFunction {
public:
    static v8::Local<v8::Function> createFunction(ScriptState* scriptState, LayoutNode* layoutNode)
    {
        ThenFunction* self = new ThenFunction(scriptState, layoutNode);
        return self->bindToV8Function();
    }

    DEFINE_INLINE_VIRTUAL_TRACE()
    {
        ScriptFunction::trace(visitor);
    }

private:
    ThenFunction(ScriptState* scriptState, LayoutNode* layoutNode)
        : ScriptFunction(scriptState)
        , m_node(layoutNode)
    {
    }

    ScriptValue call(ScriptValue value) override
    {
        m_node->setHeightInternal(value.v8Value().As<v8::Number>()->Value());
        return ScriptValue();
    }

    LayoutNode* m_node;
};

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
    //TRACE_EVENT0("blink", "LayoutNode::getCSSValue");
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

double LayoutNode::overrideWidth() const
{
    return (double) (m_renderBox->hasOverrideWidth() ? m_renderBox->overrideLogicalContentWidth().toDouble() : -1.0);
}

double LayoutNode::height() const
{
    return m_renderBox->size().height().toDouble(); // should be box size. need to add safety here.
}

void LayoutNode::setWidth(double width)
{
    LayoutUnit w(width);
    m_renderBox->setLogicalWidth(w);
}

void LayoutNode::setHeight(ScriptState* scriptState, ScriptPromise& heightPromise)
{
    WTF_LOG(NotYetImplemented, "hit: setHeight\n");
    heightPromise.then(ThenFunction::createFunction(scriptState, this));
    scriptState->isolate()->RunMicrotasks();
}

void LayoutNode::setHeightInternal(double height)
{
    WTF_LOG(NotYetImplemented, "hit: setHeightInternal, %lf\n", height);
    LayoutUnit h(height);
    m_renderBox->setLogicalHeight(h);
}

void LayoutNode::log(String str) const
{
    WTF_LOG(NotYetImplemented, "log: %s\n", str.ascii().data());
}


}
