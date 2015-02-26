// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/layout/DeadFragmentInformation.h"

#include <gtest/gtest.h>

using namespace blink;

namespace {

class DeadFragmentInformationTest : public ::testing::Test {
public:
    void testFragment(Node*, double, double, double, double, bool, size_t, DeadFragmentInformation*, DeadFragmentInformation*, DeadFragmentInformation*, DeadFragmentInformation*);

    virtual void TearDown() override;

    RefPtrWillBeMember<DeadFragmentInformation> m_fragment;
};

void DeadFragmentInformationTest::testFragment(Node* node, double width, double height, double top, double left, bool isOverflowed, size_t childrenCount, DeadFragmentInformation* nextSibling, DeadFragmentInformation* previousSibling, DeadFragmentInformation* nextInBox, DeadFragmentInformation* previousInBox)
{
    EXPECT_EQ(node, m_fragment->node());
    EXPECT_EQ(width, m_fragment->width());
    EXPECT_EQ(height, m_fragment->height());
    EXPECT_EQ(top, m_fragment->top());
    EXPECT_EQ(left, m_fragment->left());
    EXPECT_EQ(isOverflowed, m_fragment->isOverflowed());
    EXPECT_EQ(childrenCount, m_fragment->children().size());
    EXPECT_EQ(nextSibling, m_fragment->nextSibling());
    EXPECT_EQ(previousSibling, m_fragment->previousSibling());
    EXPECT_EQ(nextInBox, m_fragment->nextInBox());
    EXPECT_EQ(previousInBox, m_fragment->previousInBox());

    bool isNull;
    EXPECT_EQ(childrenCount, m_fragment->children(isNull).size());
    EXPECT_EQ(!childrenCount, isNull);

    EXPECT_EQ(nextSibling, m_fragment->nextSibling(isNull));
    EXPECT_EQ(!nextSibling, isNull);

    EXPECT_EQ(previousSibling, m_fragment->previousSibling(isNull));
    EXPECT_EQ(!previousSibling, isNull);

    EXPECT_EQ(nextInBox, m_fragment->nextInBox(isNull));
    EXPECT_EQ(!nextInBox, isNull);

    EXPECT_EQ(previousInBox, m_fragment->previousInBox(isNull));
    EXPECT_EQ(!previousInBox, isNull);
}

void DeadFragmentInformationTest::TearDown()
{
    m_fragment = nullptr;
}

TEST_F(DeadFragmentInformationTest, EmptyDeadFragmentInfoValue)
{
    m_fragment = DeadFragmentInformation::create(nullptr, 0, 0, 0, 0, false);
    testFragment(nullptr, 0, 0, 0, 0, false, 0U, nullptr, nullptr, nullptr, nullptr);
}

TEST_F(DeadFragmentInformationTest, FragmentWithSizingNoChildren)
{
    m_fragment = DeadFragmentInformation::create(nullptr, 10, 20, 30, 40, true);
    testFragment(nullptr, 10, 20, 30, 40, true, 0U, nullptr, nullptr, nullptr, nullptr);
}

TEST_F(DeadFragmentInformationTest, FragmentWithSizingAndChildren)
{
    m_fragment = DeadFragmentInformation::create(nullptr, 20, 40, 0, 0, true);
    testFragment(nullptr, 20, 40, 0, 0, true, 0U, nullptr, nullptr, nullptr, nullptr);

    // Now let's mutate.
    RefPtrWillBeMember<DeadFragmentInformation> childFragment = DeadFragmentInformation::create(nullptr, 10, 20, 30, 40, true);
    m_fragment->appendChild(childFragment.get());
    testFragment(nullptr, 20, 40, 0, 0, true, 1U, nullptr, nullptr, nullptr, nullptr);
}

// FIXME: Test all the combinaison of nextSibling, previousSibling, nextInBox, previousInBox.
TEST_F(DeadFragmentInformationTest, FragmentWithSizingAndNextSibling)
{
    m_fragment = DeadFragmentInformation::create(nullptr, 20, 40, 0, 0, true);
    testFragment(nullptr, 20, 40, 0, 0, true, 0U, nullptr, nullptr, nullptr, nullptr);

    // Now let's mutate.
    RefPtrWillBeMember<DeadFragmentInformation> nextSiblingFragment = DeadFragmentInformation::create(nullptr, 10, 20, 30, 40, true);
    m_fragment->setNextSibling(nextSiblingFragment.get());
    testFragment(nullptr, 20, 40, 0, 0, true, 0U, nextSiblingFragment.get(), nullptr, nullptr, nullptr);
}

TEST_F(DeadFragmentInformationTest, FragmentWithSizingAndPreviousSibling)
{
    m_fragment = DeadFragmentInformation::create(nullptr, 20, 40, 0, 0, true);
    testFragment(nullptr, 20, 40, 0, 0, true, 0U, nullptr, nullptr, nullptr, nullptr);

    // Now let's mutate.
    RefPtrWillBeMember<DeadFragmentInformation> previousSiblingFragment = DeadFragmentInformation::create(nullptr, 10, 20, 30, 40, true);
    m_fragment->setPreviousSibling(previousSiblingFragment.get());
    testFragment(nullptr, 20, 40, 0, 0, true, 0U, nullptr, previousSiblingFragment.get(), nullptr, nullptr);
}

TEST_F(DeadFragmentInformationTest, FragmentWithSizingAndNextInBox)
{
    m_fragment = DeadFragmentInformation::create(nullptr, 20, 40, 0, 0, true);
    testFragment(nullptr, 20, 40, 0, 0, true, 0U, nullptr, nullptr, nullptr, nullptr);

    // Now let's mutate.
    RefPtrWillBeMember<DeadFragmentInformation> nextInBoxFragment = DeadFragmentInformation::create(nullptr, 10, 20, 30, 40, true);
    m_fragment->setNextInBox(nextInBoxFragment.get());
    testFragment(nullptr, 20, 40, 0, 0, true, 0U, nullptr, nullptr, nextInBoxFragment.get(), nullptr);
}

TEST_F(DeadFragmentInformationTest, FragmentWithSizingAndPreviousInBox)
{
    m_fragment = DeadFragmentInformation::create(nullptr, 20, 40, 0, 0, true);
    testFragment(nullptr, 20, 40, 0, 0, true, 0U, nullptr, nullptr, nullptr, nullptr);

    // Now let's mutate.
    RefPtrWillBeMember<DeadFragmentInformation> previousInBoxFragment = DeadFragmentInformation::create(nullptr, 10, 20, 30, 40, true);
    m_fragment->setPreviousInBox(previousInBoxFragment.get());
    testFragment(nullptr, 20, 40, 0, 0, true, 0U, nullptr, nullptr, nullptr, previousInBoxFragment.get());
}

}
