/*
 * Copyright (C) 2006 Apple Computer, Inc.
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010-2012. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef RenderSVGText_h
#define RenderSVGText_h

#include "core/layout/svg/SVGTextLayoutAttributesBuilder.h"
#include "core/rendering/svg/RenderSVGBlock.h"
#include "platform/transforms/AffineTransform.h"

namespace blink {

class RenderSVGInlineText;
class SVGTextElement;
class RenderSVGInlineText;

class RenderSVGText final : public RenderSVGBlock {
public:
    explicit RenderSVGText(SVGTextElement*);
    virtual ~RenderSVGText();

    virtual bool isChildAllowed(LayoutObject*, const LayoutStyle&) const override;

    void setNeedsPositioningValuesUpdate() { m_needsPositioningValuesUpdate = true; }
    virtual void setNeedsTransformUpdate() override { m_needsTransformUpdate = true; }
    void setNeedsTextMetricsUpdate() { m_needsTextMetricsUpdate = true; }
    virtual FloatRect paintInvalidationRectInLocalCoordinates() const override;

    static RenderSVGText* locateRenderSVGTextAncestor(LayoutObject*);
    static const RenderSVGText* locateRenderSVGTextAncestor(const LayoutObject*);

    bool needsReordering() const { return m_needsReordering; }
    Vector<SVGTextLayoutAttributes*>& layoutAttributes() { return m_layoutAttributes; }

    void subtreeChildWasAdded(LayoutObject*);
    void subtreeChildWillBeRemoved(LayoutObject*, Vector<SVGTextLayoutAttributes*, 2>& affectedAttributes);
    void subtreeChildWasRemoved(const Vector<SVGTextLayoutAttributes*, 2>& affectedAttributes);
    void subtreeStyleDidChange();
    void subtreeTextDidChange(RenderSVGInlineText*);

    virtual const AffineTransform& localToParentTransform() const override { return m_localTransform; }

private:
    virtual const char* renderName() const override { return "RenderSVGText"; }
    virtual bool isOfType(LayoutObjectType type) const override { return type == LayoutObjectSVGText || RenderSVGBlock::isOfType(type); }

    virtual void paint(const PaintInfo&, const LayoutPoint&) override;
    virtual bool nodeAtFloatPoint(const HitTestRequest&, HitTestResult&, const FloatPoint& pointInParent, HitTestAction) override;
    virtual PositionWithAffinity positionForPoint(const LayoutPoint&) override;

    virtual void layout() override;

    virtual void absoluteQuads(Vector<FloatQuad>&, bool* wasFixed) const override;

    virtual void addChild(LayoutObject* child, LayoutObject* beforeChild = 0) override;
    virtual void removeChild(LayoutObject*) override;
    virtual void willBeDestroyed() override;

    virtual FloatRect objectBoundingBox() const override { return frameRect(); }
    virtual FloatRect strokeBoundingBox() const override;

    virtual RootInlineBox* createRootInlineBox() override;

    bool shouldHandleSubtreeMutations() const;

    bool m_needsReordering : 1;
    bool m_needsPositioningValuesUpdate : 1;
    bool m_needsTransformUpdate : 1;
    bool m_needsTextMetricsUpdate : 1;
    SVGTextLayoutAttributesBuilder m_layoutAttributesBuilder;
    Vector<SVGTextLayoutAttributes*> m_layoutAttributes;
};

DEFINE_LAYOUT_OBJECT_TYPE_CASTS(RenderSVGText, isSVGText());

}

#endif
