#ifndef RenderCustomBox_h
#define RenderCustomBox_h

#include "core/rendering/RenderBlock.h"

namespace blink {

class RenderCustomBox : public RenderBlock {
public:
    RenderCustomBox(Element*);
    virtual ~RenderCustomBox();

    virtual const char* renderName() const override { return "RenderCustomBox"; }
    virtual bool isCustomBox() const override final { return true; }
    virtual bool canCollapseAnonymousBlockChild() const override { return false; }

    virtual void layoutBlock(bool relayoutChildren) override;

protected:
    virtual void computeIntrinsicLogicalWidths(LayoutUnit& minLogicalWidth, LayoutUnit& maxLogicalWidth) const override;
};

} // namespace blink

#endif // RenderCustomBox_h
