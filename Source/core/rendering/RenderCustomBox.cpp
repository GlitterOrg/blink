#include "config.h"
#include "core/customlayout/LayoutWorker.h"
#include "core/rendering/RenderCustomBox.h"
#include "wtf/Vector.h"

#include "core/layout/TextAutosizer.h"
#include "core/layout/LayoutState.h"
#include "platform/LengthFunctions.h"
#include "platform/Logging.h"

namespace blink {

RenderCustomBox::RenderCustomBox(Element* element)
    : RenderBlock(element)
{
}

RenderCustomBox::~RenderCustomBox()
{
}

void RenderCustomBox::computeIntrinsicLogicalWidths(LayoutUnit& minLogicalWidth, LayoutUnit& maxLogicalWidth) const
{
    Vector<RenderBox*> children;

    for (RenderBox* child = firstChildBox(); child; child = child->nextSiblingBox()) {
        children.append(child);
    }

    toElement(node())->layout()->computeIntrinsicLogicalWidths(minLogicalWidth, maxLogicalWidth, children /*, style */);

    WTF_LOG(NotYetImplemented, "minLogicalWidth: %lf\n", minLogicalWidth.toDouble());
    WTF_LOG(NotYetImplemented, "maxLogicalWidth: %lf\n", maxLogicalWidth.toDouble());
}

void RenderCustomBox::layoutBlock(bool relayoutChildren)
{
    ASSERT(needsLayout());

    if (!relayoutChildren && simplifiedLayout())
        return;

    updateLogicalWidthAndColumnWidth();

    LayoutUnit previousHeight = logicalHeight();
    setLogicalHeight(borderAndPaddingLogicalHeight() + scrollbarLogicalHeight());

    {
        TextAutosizer::LayoutScope textAutosizerLayoutScope(this);
        LayoutState state(*this, locationOffset());

        layoutCustomItems(/* relayoutChildren */);

        if (logicalHeight() != previousHeight)
            relayoutChildren = true;
    }

    updateLayerTransformAfterLayout();

    updateScrollInfoAfterLayout();

    clearNeedsLayout();
}

void RenderCustomBox::layoutCustomItems()
{
    Vector<RenderBox*> children;

    // TODO
    for (RenderBox* child = firstChildBox(); child; child = child->nextSiblingBox()) {
        child->layoutIfNeeded();
        child->setLocation(LayoutPoint(0, 0));

        children.append(child);
    }

    LayoutUnit height(0);
    toElement(node())->layout()->calculateHeightAndPositionChildren(height, children);

    setLogicalHeight(height);
    updateLogicalHeight();
}

} // namespace blink
