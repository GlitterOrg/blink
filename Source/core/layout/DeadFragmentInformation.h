// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeadFragmentInformation_h
#define DeadFragmentInformation_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "wtf/RefCounted.h"

namespace blink {

class DeadFragmentInformation : public RefCountedWillBeGarbageCollected<DeadFragmentInformation>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    typedef WillBeHeapVector<RefPtrWillBeMember<DeadFragmentInformation>> DeadFragmentInformationVector;

    static RefPtrWillBeMember<DeadFragmentInformation> create(Node* node, double width, double height, double top, double left, bool isOverflowed)
    {
        return adoptRef(new DeadFragmentInformation(node, width, height, top, left, isOverflowed));
    }

    Node* node() const { return m_node; }
    double width() const { return m_width; }
    double height() const { return m_height; }
    double top() const { return m_top; }
    double left() const { return m_left; }
    bool isOverflowed() const { return m_isOverflowed; }
    const DeadFragmentInformationVector& children(bool& isNull) const { isNull = m_children.isEmpty(); return m_children; }
    const DeadFragmentInformationVector& children() const { return m_children; }
    DeadFragmentInformation* nextSibling(bool& isNull) const { isNull = !m_nextSibling; return m_nextSibling; }
    DeadFragmentInformation* nextSibling() const { return m_nextSibling; }
    DeadFragmentInformation* previousSibling(bool& isNull) const { isNull = !m_previousSibling; return m_previousSibling; }
    DeadFragmentInformation* previousSibling() const { return m_previousSibling; }
    DeadFragmentInformation* nextInBox(bool& isNull) const { isNull = !m_nextInBox; return m_nextInBox; }
    DeadFragmentInformation* nextInBox() const { return m_nextInBox; }
    DeadFragmentInformation* previousInBox(bool& isNull) const { isNull = !m_previousInBox; return m_previousInBox; }
    DeadFragmentInformation* previousInBox() const { return m_previousInBox; }

    // These functions are not part of the JS API (all the attributes are readonly in the IDL).
    void setNextSibling(DeadFragmentInformation* nextSibling) { m_nextSibling = nextSibling; }
    void setPreviousSibling(DeadFragmentInformation* previousSibling) { m_previousSibling = previousSibling; }
    void setNextInBox(DeadFragmentInformation* nextInBox) { m_nextInBox = nextInBox; }
    void setPreviousInBox(DeadFragmentInformation* previousInBox) { m_previousInBox = previousInBox; }
    void appendChild(DeadFragmentInformation* child) { m_children.append(child); }

private:
    DeadFragmentInformation(Node* node, double width, double height, double top, double left, double isOverflowed)
        : m_node(node)
        , m_width(width)
        , m_height(height)
        , m_top(top)
        , m_left(left)
        , m_isOverflowed(isOverflowed)
        , m_nextSibling(nullptr)
        , m_previousSibling(nullptr)
        , m_nextInBox(nullptr)
        , m_previousInBox(nullptr)
    {
    }

    DeadFragmentInformationVector m_children;
    Node* m_node;
    double m_width;
    double m_height;
    double m_top;
    double m_left;
    bool m_isOverflowed;
    DeadFragmentInformation* m_nextSibling;
    DeadFragmentInformation* m_previousSibling;
    DeadFragmentInformation* m_nextInBox;
    DeadFragmentInformation* m_previousInBox;
};

}

#endif // DeadFragmentInformation_h
