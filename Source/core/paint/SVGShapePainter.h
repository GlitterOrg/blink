// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SVGShapePainter_h
#define SVGShapePainter_h

namespace blink {

struct MarkerPosition;
struct PaintInfo;
class FloatPoint;
class GraphicsContext;
class Path;
class LayoutSVGResourceMarker;
class RenderSVGShape;

class SVGShapePainter {
public:
    SVGShapePainter(RenderSVGShape& renderSVGShape) : m_renderSVGShape(renderSVGShape) { }

    void paint(const PaintInfo&);

private:
    void fillShape(GraphicsContext*);
    void strokeShape(GraphicsContext*);

    void paintMarkers(const PaintInfo&);
    void paintMarker(const PaintInfo&, LayoutSVGResourceMarker&, const MarkerPosition&, float);
    void strokeZeroLengthLineCaps(GraphicsContext*);
    Path* zeroLengthLinecapPath(const FloatPoint&) const;

    RenderSVGShape& m_renderSVGShape;
};

} // namespace blink

#endif // SVGShapePainter_h
