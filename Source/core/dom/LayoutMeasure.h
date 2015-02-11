// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LayoutMeasure_h
#define LayoutMeasure_h

#include "bindings/core/v8/ScriptWrappable.h"

namespace blink {

class LayoutMeasure : public GarbageCollected<LayoutMeasure>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static LayoutMeasure* create(float width, float height) { return new LayoutMeasure(width, height); }

    float width() const { return m_width; }
    float height() const { return m_height; }

    void trace(Visitor*) { }

private:
    LayoutMeasure(float width, float height)
        : m_width(width)
        , m_height(height)
    {
    }

    float m_width;
    float m_height;
};

} // namespace blink

#endif // LayoutMeasure_h
