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
    TRACE_EVENT0("blink", "RenderCustomBox::layoutBlock");
    ASSERT(needsLayout());

    if (!relayoutChildren && simplifiedLayout())
        return;

    LayoutWorker* layout = toElement(node())->layout();
    LayoutUnit previousHeight = logicalHeight();

    {
        TextAutosizer::LayoutScope textAutosizerLayoutScope(this);
        LayoutState state(*this, locationOffset());

        layout->doLayout(*this);

        if (logicalHeight() != previousHeight)
            relayoutChildren = true;
    }

    updateLayerTransformAfterLayout();

    updateScrollInfoAfterLayout();

    clearNeedsLayout();
}

} // namespace blink
