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
    toElement(node())->layout()->computeIntrinsicLogicalWidths(minLogicalWidth, maxLogicalWidth, const_cast<RenderCustomBox&>(*this));

    WTF_LOG(NotYetImplemented, "minLogicalWidth: %lf\n", minLogicalWidth.toDouble());
    WTF_LOG(NotYetImplemented, "maxLogicalWidth: %lf\n", maxLogicalWidth.toDouble());
}

void RenderCustomBox::layoutBlock(bool relayoutChildren)
{
    ASSERT(needsLayout());

    if (!relayoutChildren && simplifiedLayout())
        return;

    LayoutWorker* layout = toElement(node())->layout();

    // Calculate & update the width.
    LayoutUnit width(0);
    layout->calculateWidth(width, *this);
    setLogicalWidth(width);

    LayoutUnit previousHeight = logicalHeight();
    setLogicalHeight(borderAndPaddingLogicalHeight() + scrollbarLogicalHeight());

    {
        TextAutosizer::LayoutScope textAutosizerLayoutScope(this);
        LayoutState state(*this, locationOffset());

        // Layout children here, just incase we don't have it done.
        for (RenderBox* child = firstChildBox(); child; child = child->nextSiblingBox()) {
            child->layoutIfNeeded();
            child->setLocation(LayoutPoint(0, 0));
        }

        // Calculate height... note constraints may be placed on children in
        // this phase?
        LayoutUnit height(0);
        layout->calculateHeight(height, *this);
        setLogicalHeight(height);

        // Position children.
        layout->positionChildren(*this);

        if (logicalHeight() != previousHeight)
            relayoutChildren = true;
    }

    updateLayerTransformAfterLayout();

    updateScrollInfoAfterLayout();

    clearNeedsLayout();
}

} // namespace blink
