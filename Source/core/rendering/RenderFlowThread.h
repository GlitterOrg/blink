/*
 * Copyright (C) 2011 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef RenderFlowThread_h
#define RenderFlowThread_h

#include "core/layout/LayerFragment.h"
#include "core/rendering/RenderBlockFlow.h"
#include "wtf/ListHashSet.h"

namespace blink {

class RenderMultiColumnSet;
class RenderRegion;

typedef ListHashSet<RenderMultiColumnSet*> RenderMultiColumnSetList;

// RenderFlowThread is used to collect all the render objects that participate in a
// flow thread. It will also help in doing the layout. However, it will not render
// directly to screen. Instead, RenderRegion objects will redirect their paint
// and nodeAtPoint methods to this object. Each RenderRegion will actually be a viewPort
// of the RenderFlowThread.

class RenderFlowThread: public RenderBlockFlow {
public:
    RenderFlowThread();
    virtual ~RenderFlowThread() { };

    virtual bool isRenderFlowThread() const override final { return true; }
    virtual bool isRenderMultiColumnFlowThread() const { return false; }
    virtual bool isRenderPagedFlowThread() const { return false; }

    virtual bool supportsPaintInvalidationStateCachedOffsets() const override { return false; }

    virtual void layout() override;

    // Always create a Layer for the RenderFlowThread so that we
    // can easily avoid drawing the children directly.
    virtual LayerType layerTypeRequired() const override final { return NormalLayer; }

    // Skip past a column spanner during flow thread layout. Spanners are not laid out inside the
    // flow thread, since the flow thread is not in a spanner's containing block chain (since the
    // containing block is the multicol container). If the spanner follows right after a column set
    // (as opposed to following another spanner), we may have to stretch the flow thread to ensure
    // completely filled columns in the preceding column set. Return this adjustment, if any.
    virtual LayoutUnit skipColumnSpanner(RenderBox*, LayoutUnit logicalTopInFlowThread) { return LayoutUnit(); }

    virtual void flowThreadDescendantWasInserted(LayoutObject*) { }
    virtual void flowThreadDescendantWillBeRemoved(LayoutObject*) { }

    virtual bool nodeAtPoint(const HitTestRequest&, HitTestResult&, const HitTestLocation& locationInContainer, const LayoutPoint& accumulatedOffset, HitTestAction) override final;

    virtual void addRegionToThread(RenderMultiColumnSet*) = 0;
    virtual void removeRegionFromThread(RenderMultiColumnSet*);

    virtual void computeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop, LogicalExtentComputedValues&) const override;

    bool hasRegions() const { return m_multiColumnSetList.size(); }

    void validateRegions();
    void invalidateRegions();
    bool hasValidRegionInfo() const { return !m_regionsInvalidated && !m_multiColumnSetList.isEmpty(); }

    virtual void mapRectToPaintInvalidationBacking(const LayoutLayerModelObject* paintInvalidationContainer, LayoutRect&, const PaintInvalidationState*) const override;

    LayoutUnit pageLogicalHeightForOffset(LayoutUnit);
    LayoutUnit pageRemainingLogicalHeightForOffset(LayoutUnit, PageBoundaryRule = IncludePageBoundary);

    virtual void setPageBreak(LayoutUnit /*offset*/, LayoutUnit /*spaceShortage*/) { }
    virtual void updateMinimumPageHeight(LayoutUnit /*offset*/, LayoutUnit /*minHeight*/) { }

    bool regionsHaveUniformLogicalHeight() const { return m_regionsHaveUniformLogicalHeight; }

    // FIXME: These 2 functions should return a RenderMultiColumnSet.
    RenderRegion* firstRegion() const;
    RenderRegion* lastRegion() const;

    virtual bool addForcedRegionBreak(LayoutUnit, LayoutObject* breakChild, bool isBefore, LayoutUnit* offsetBreakAdjustment = 0) { return false; }

    virtual bool isPageLogicalHeightKnown() const { return true; }
    bool pageLogicalSizeChanged() const { return m_pageLogicalSizeChanged; }

    void collectLayerFragments(LayerFragments&, const LayoutRect& layerBoundingBox, const LayoutRect& dirtyRect);
    LayoutRect fragmentsBoundingBox(const LayoutRect& layerBoundingBox) const;

    LayoutPoint flowThreadPointToVisualPoint(const LayoutPoint& flowThreadPoint) const
    {
        return flowThreadPoint + columnOffset(flowThreadPoint);
    }

    // Used to estimate the maximum height of the flow thread.
    static LayoutUnit maxLogicalHeight() { return LayoutUnit::max() / 2; }

    virtual RenderMultiColumnSet* columnSetAtBlockOffset(LayoutUnit) const = 0;

protected:
    virtual const char* renderName() const = 0;

    void updateRegionsFlowThreadPortionRect();

    RenderMultiColumnSetList m_multiColumnSetList;

    typedef PODInterval<LayoutUnit, RenderMultiColumnSet*> MultiColumnSetInterval;
    typedef PODIntervalTree<LayoutUnit, RenderMultiColumnSet*> MultiColumnSetIntervalTree;

    class MultiColumnSetSearchAdapter {
    public:
        MultiColumnSetSearchAdapter(LayoutUnit offset)
            : m_offset(offset)
            , m_result(0)
        {
        }

        const LayoutUnit& lowValue() const { return m_offset; }
        const LayoutUnit& highValue() const { return m_offset; }
        void collectIfNeeded(const MultiColumnSetInterval&);

        RenderMultiColumnSet* result() const { return m_result; }

    private:
        LayoutUnit m_offset;
        RenderMultiColumnSet* m_result;
    };

    MultiColumnSetIntervalTree m_multiColumnSetIntervalTree;

    bool m_regionsInvalidated : 1;
    bool m_regionsHaveUniformLogicalHeight : 1;
    bool m_pageLogicalSizeChanged : 1;
};

DEFINE_LAYOUT_OBJECT_TYPE_CASTS(RenderFlowThread, isRenderFlowThread());

// These structures are used by PODIntervalTree for debugging.
#ifndef NDEBUG
template <> struct ValueToString<LayoutUnit> {
    static String string(const LayoutUnit value) { return String::number(value.toFloat()); }
};

template <> struct ValueToString<RenderMultiColumnSet*> {
    static String string(const RenderMultiColumnSet* value) { return String::format("%p", value); }
};
#endif

} // namespace blink

#endif // RenderFlowThread_h
