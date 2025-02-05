/*
 * Copyright (C) 2013 Google Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PageLifecycleNotifier_h
#define PageLifecycleNotifier_h

#include "core/page/Page.h"
#include "core/page/PageLifecycleObserver.h"
#include "platform/LifecycleNotifier.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/TemporaryChange.h"

namespace blink {

class LocalFrame;

class PageLifecycleNotifier final : public LifecycleNotifier<Page> {
public:
    static PassOwnPtr<PageLifecycleNotifier> create(Page*);

    void notifyPageVisibilityChanged();
    void notifyDidCommitLoad(LocalFrame*);

    virtual void addObserver(Observer*) override;
    virtual void removeObserver(Observer*) override;

private:
    explicit PageLifecycleNotifier(Page*);

    using PageObserverSet = HashSet<PageLifecycleObserver*>;
    PageObserverSet m_pageObservers;
};

inline PassOwnPtr<PageLifecycleNotifier> PageLifecycleNotifier::create(Page* context)
{
    return adoptPtr(new PageLifecycleNotifier(context));
}

inline void PageLifecycleNotifier::notifyPageVisibilityChanged()
{
    TemporaryChange<IterationType> scope(this->m_iterating, IteratingOverPageObservers);
    for (PageLifecycleObserver* pageObserver : m_pageObservers)
        pageObserver->pageVisibilityChanged();
}

inline void PageLifecycleNotifier::notifyDidCommitLoad(LocalFrame* frame)
{
    TemporaryChange<IterationType> scope(this->m_iterating, IteratingOverPageObservers);
    for (PageLifecycleObserver* pageObserver : m_pageObservers)
        pageObserver->didCommitLoad(frame);
}

} // namespace blink

#endif // PageLifecycleNotifier_h
