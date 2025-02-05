/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Samsung Electronics. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ImageInputType_h
#define ImageInputType_h

#include "core/html/forms/BaseButtonInputType.h"
#include "platform/geometry/IntPoint.h"

namespace blink {

class ImageInputType final : public BaseButtonInputType {
public:
    static PassRefPtrWillBeRawPtr<InputType> create(HTMLInputElement&);
    virtual PassRefPtr<LayoutStyle> customStyleForRenderer(PassRefPtr<LayoutStyle>);

private:
    ImageInputType(HTMLInputElement&);
    virtual const AtomicString& formControlType() const override;
    virtual bool isFormDataAppendable() const override;
    virtual bool appendFormData(FormDataList&, bool) const override;
    virtual String resultForDialogSubmit() const override;
    virtual bool supportsValidation() const override;
    virtual LayoutObject* createRenderer(const LayoutStyle&) const override;
    virtual void handleDOMActivateEvent(Event*) override;
    virtual void altAttributeChanged() override;
    virtual void srcAttributeChanged() override;
    virtual void valueAttributeChanged() override;
    virtual void startResourceLoading() override;
    virtual bool shouldRespectAlignAttribute() override;
    virtual bool canBeSuccessfulSubmitButton() override;
    virtual bool isEnumeratable() override;
    virtual bool isImage() const override;
    virtual bool shouldRespectHeightAndWidthAttributes() override;
    virtual unsigned height() const override;
    virtual unsigned width() const override;
    virtual bool hasLegalLinkAttribute(const QualifiedName&) const override;
    virtual const QualifiedName& subResourceAttributeName() const override;
    virtual void ensureFallbackContent() override;
    virtual void ensurePrimaryContent() override;
    virtual void createShadowSubtree() override;

    void reattachFallbackContent();
    void setUseFallbackContent();
    bool hasFallbackContent() const { return m_useFallbackContent; }

    IntPoint m_clickLocation; // Valid only during HTMLFormElement::prepareForSubmission().
    bool m_useFallbackContent;
};

} // namespace blink

#endif // ImageInputType_h
