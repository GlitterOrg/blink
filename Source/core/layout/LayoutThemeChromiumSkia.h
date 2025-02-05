/*
 * This file is part of the WebKit project.
 *
 * Copyright (C) 2006 Apple Computer, Inc.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008, 2009 Google, Inc.
 * All rights reserved.
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
 *
 */

#ifndef LayoutThemeChromiumSkia_h
#define LayoutThemeChromiumSkia_h

#include "core/layout/LayoutTheme.h"

namespace blink {

class RenderProgress;

class LayoutThemeChromiumSkia : public LayoutTheme {
public:
    LayoutThemeChromiumSkia();
    virtual ~LayoutThemeChromiumSkia();

    virtual String extraDefaultStyleSheet() override;
    virtual String extraQuirksStyleSheet() override;

    virtual Color platformTapHighlightColor() const override
    {
        return Color(defaultTapHighlightColor);
    }

    // A method asking if the theme's controls actually care about redrawing when hovered.
    virtual bool supportsHover(const LayoutStyle&) const override final;

    // A method asking if the theme is able to draw the focus ring.
    virtual bool supportsFocusRing(const LayoutStyle&) const override;

    // The platform selection color.
    virtual Color platformActiveSelectionBackgroundColor() const override;
    virtual Color platformInactiveSelectionBackgroundColor() const override;
    virtual Color platformActiveSelectionForegroundColor() const override;
    virtual Color platformInactiveSelectionForegroundColor() const override;
    virtual Color platformFocusRingColor() const override;

    // To change the blink interval, override caretBlinkIntervalInternal instead of this one so that we may share layout test code an intercepts.
    virtual double caretBlinkInterval() const override;

    // System fonts.
    virtual void systemFont(CSSValueID systemFontID, FontStyle&, FontWeight&, float& fontSize, AtomicString& fontFamily) const;

    virtual int minimumMenuListSize(const LayoutStyle&) const override;

    virtual void setCheckboxSize(LayoutStyle&) const override;

    virtual void setRadioSize(LayoutStyle&) const override;

    virtual void adjustButtonStyle(LayoutStyle&, Element*) const override;

    virtual bool paintTextArea(LayoutObject*, const PaintInfo&, const IntRect&) override;

    virtual void adjustSearchFieldStyle(LayoutStyle&, Element*) const override;
    virtual bool paintSearchField(LayoutObject*, const PaintInfo&, const IntRect&) override;

    virtual void adjustSearchFieldCancelButtonStyle(LayoutStyle&, Element*) const override;
    virtual bool paintSearchFieldCancelButton(LayoutObject*, const PaintInfo&, const IntRect&) override;

    virtual void adjustSearchFieldDecorationStyle(LayoutStyle&, Element*) const override;

    virtual void adjustSearchFieldResultsDecorationStyle(LayoutStyle&, Element*) const override;
    virtual bool paintSearchFieldResultsDecoration(LayoutObject*, const PaintInfo&, const IntRect&) override;

    virtual bool paintMediaSliderTrack(LayoutObject*, const PaintInfo&, const IntRect&) override;
    virtual bool paintMediaVolumeSliderTrack(LayoutObject*, const PaintInfo&, const IntRect&) override;
    virtual void adjustSliderThumbSize(LayoutStyle&, Element*) const override;
    virtual bool paintMediaSliderThumb(LayoutObject*, const PaintInfo&, const IntRect&) override;
    virtual bool paintMediaToggleClosedCaptionsButton(LayoutObject*, const PaintInfo&, const IntRect&) override;
    virtual bool paintMediaCastButton(LayoutObject*, const PaintInfo&, const IntRect&) override;
    virtual bool paintMediaVolumeSliderThumb(LayoutObject*, const PaintInfo&, const IntRect&) override;
    virtual bool paintMediaPlayButton(LayoutObject*, const PaintInfo&, const IntRect&) override;
    virtual bool paintMediaOverlayPlayButton(LayoutObject*, const PaintInfo&, const IntRect&) override;
    virtual bool paintMediaMuteButton(LayoutObject*, const PaintInfo&, const IntRect&) override;
    virtual String formatMediaControlsTime(float time) const override;
    virtual String formatMediaControlsCurrentTime(float currentTime, float duration) const override;
    virtual bool paintMediaFullscreenButton(LayoutObject*, const PaintInfo&, const IntRect&) override;

    // MenuList refers to an unstyled menulist (meaning a menulist without
    // background-color or border set) and MenuListButton refers to a styled
    // menulist (a menulist with background-color or border set). They have
    // this distinction to support showing aqua style themes whenever they
    // possibly can, which is something we don't want to replicate.
    //
    // In short, we either go down the MenuList code path or the MenuListButton
    // codepath. We never go down both. And in both cases, they render the
    // entire menulist.
    virtual void adjustMenuListStyle(LayoutStyle&, Element*) const override;
    virtual void adjustMenuListButtonStyle(LayoutStyle&, Element*) const override;
    virtual bool paintMenuListButton(LayoutObject*, const PaintInfo&, const IntRect&) override;

    virtual double animationRepeatIntervalForProgressBar(RenderProgress*) const override;
    virtual double animationDurationForProgressBar(RenderProgress*) const override;

    // These methods define the padding for the MenuList's inner block.
    virtual int popupInternalPaddingLeft(const LayoutStyle&) const override;
    virtual int popupInternalPaddingRight(const LayoutStyle&) const override;
    virtual int popupInternalPaddingTop(const LayoutStyle&) const override;
    virtual int popupInternalPaddingBottom(const LayoutStyle&) const override;

    // Provide a way to pass the default font size from the Settings object
    // to the render theme. FIXME: http://b/1129186 A cleaner way would be
    // to remove the default font size from this object and have callers
    // that need the value to get it directly from the appropriate Settings
    // object.
    static void setDefaultFontSize(int);

protected:
    virtual double caretBlinkIntervalInternal() const;

    virtual int menuListArrowPadding() const;

    IntRect determinateProgressValueRectFor(RenderProgress*, const IntRect&) const;
    IntRect indeterminateProgressValueRectFor(RenderProgress*, const IntRect&) const;
    IntRect progressValueRectFor(RenderProgress*, const IntRect&) const;

    class DirectionFlippingScope {
    public:
        DirectionFlippingScope(LayoutObject*, const PaintInfo&, const IntRect&);
        ~DirectionFlippingScope();

    private:
        bool m_needsFlipping;
        const PaintInfo& m_paintInfo;
    };

private:
    virtual bool shouldShowPlaceholderWhenFocused() const override;

    int menuListInternalPadding(const LayoutStyle&, int paddingType) const;
    bool paintMediaButtonInternal(GraphicsContext*, const IntRect&, Image*);
    IntRect convertToPaintingRect(LayoutObject* inputRenderer, const LayoutObject* partRenderer, LayoutRect partRect, const IntRect& localOffset) const;

    static const RGBA32 defaultTapHighlightColor = 0x2e000000; // 18% black.
};

} // namespace blink

#endif // LayoutThemeChromiumSkia_h
