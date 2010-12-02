/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */
#ifndef MIMABSTRACTKEYAREASTYLE_H
#define MIMABSTRACTKEYAREASTYLE_H

#include <MWidgetStyle>

/*!
    \brief Style for MImAbstractKeyArea derived classes.
*/
class M_EXPORT MImAbstractKeyAreaStyle : public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(MImAbstractKeyAreaStyle)

public:
    M_STYLE_ATTRIBUTE(QColor, fontColor, FontColor)
    M_STYLE_ATTRIBUTE(QFont, font, Font)
    M_STYLE_ATTRIBUTE(QFont, secondaryFont, SecondaryFont)

    M_STYLE_ATTRIBUTE(int, labelMarginTop, LabelMarginTop)
    M_STYLE_ATTRIBUTE(int, labelMarginLeftWithSecondary, LabelMarginLeftWithSecondary)
    M_STYLE_ATTRIBUTE(int, secondaryLabelSeparation, SecondaryLabelSeparation)

    M_STYLE_ATTRIBUTE(int, longPressTimeout, LongPressTimeout)
    M_STYLE_ATTRIBUTE(int, idleVkbTimeout, IdleVkbTimeout)
    M_STYLE_ATTRIBUTE(int, flickGestureTimeout, FlickGestureTimeout)

    M_STYLE_ATTRIBUTE(qreal, flickGestureThresholdRatio, FlickGestureThresholdRatio)
    M_STYLE_ATTRIBUTE(qreal, touchpointHorizontalGravity, TouchpointHorizontalGravity)
    M_STYLE_ATTRIBUTE(qreal, touchpointVerticalGravity, TouchpointVerticalGravity)

    M_STYLE_ATTRIBUTE(QSize, size, Size)
    M_STYLE_ATTRIBUTE(qreal, keyHeightSmall, KeyHeightSmall)
    M_STYLE_ATTRIBUTE(qreal, keyHeightMedium, KeyHeightMedium)
    M_STYLE_ATTRIBUTE(qreal, keyHeightLarge,  KeyHeightLarge)
    M_STYLE_ATTRIBUTE(qreal, keyHeightXLarge, KeyHeightXLarge)
    M_STYLE_ATTRIBUTE(qreal, keyHeightXxLarge, KeyHeightXxLarge)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackground, KeyBackground)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundPressed, KeyBackgroundPressed)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSelected, KeyBackgroundSelected)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundPressedSelected, KeyBackgroundPressedSelected)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecial, KeyBackgroundSpecial)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialPressed, KeyBackgroundSpecialPressed)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialSelected, KeyBackgroundSpecialSelected)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialPressedSelected, KeyBackgroundSpecialPressedSelected)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkey, KeyBackgroundDeadkey)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeyPressed, KeyBackgroundDeadkeyPressed)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeySelected, KeyBackgroundDeadkeySelected)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeyPressedSelected, KeyBackgroundDeadkeyPressedSelected)

    M_STYLE_ATTRIBUTE(bool, useFixedKeyWidth, UseFixedKeyWidth)

    M_STYLE_ATTRIBUTE(qreal, keyWidthSmall, KeyWidthSmall)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthSmallFixed, KeyWidthSmallFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthMedium, KeyWidthMedium)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthMediumFixed, KeyWidthMediumFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthLarge,  KeyWidthLarge)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthLargeFixed,  KeyWidthLargeFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthXLarge, KeyWidthXLarge)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthXLargeFixed, KeyWidthXLargeFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthXxLarge, KeyWidthXxLarge)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthXxLargeFixed, KeyWidthXxLargeFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthStretched, KeyWidthStretched)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthStretchedFixed, KeyWidthStretchedFixed)

    M_STYLE_ATTRIBUTE(QSize,   keyBackspaceIconSize,    KeyBackspaceIconSize)
    M_STYLE_ATTRIBUTE(QString, keyBackspaceIconId,      KeyBackspaceIconId)
    M_STYLE_ATTRIBUTE(QString, keyBackspaceIconIdRtl,   KeyBackspaceIconIdRtl)

    M_STYLE_ATTRIBUTE(QSize, keyMenuIconSize, KeyMenuIconSize)
    M_STYLE_ATTRIBUTE(QString, keyMenuIconId, KeyMenuIconId)
    M_STYLE_ATTRIBUTE(QString, keyMenuIconIdRtl, KeyMenuIconIdRtl)

    M_STYLE_ATTRIBUTE(QSize, keyEnterIconSize, KeyEnterIconSize)
    M_STYLE_ATTRIBUTE(QString, keyEnterIconId, KeyEnterIconId)
    M_STYLE_ATTRIBUTE(QString, keyEnterIconIdRtl, KeyEnterIconIdRtl)

    M_STYLE_ATTRIBUTE(QSize, keyTabIconSize, KeyTabIconSize)
    M_STYLE_ATTRIBUTE(QString, keyTabIconId, KeyTabIconId)

    M_STYLE_ATTRIBUTE(QSize, keyShiftIconSize, KeyShiftIconSize)
    M_STYLE_ATTRIBUTE(QString, keyShiftIconId, KeyShiftIconId)
    M_STYLE_ATTRIBUTE(QString, keyShiftUppercaseIconId, KeyShiftUppercaseIconId)

    M_STYLE_ATTRIBUTE(bool, drawButtonBoundingRects, DrawButtonBoundingRects)
    M_STYLE_ATTRIBUTE(bool, drawButtonRects, DrawButtonRects)
    M_STYLE_ATTRIBUTE(bool, debugTouchPoints, DebugTouchPoints)
    M_STYLE_ATTRIBUTE(bool, drawReactiveAreas, drawReactiveAreas)

    M_STYLE_ATTRIBUTE(qreal, keyMarginLeft, KeyMarginLeft)
    M_STYLE_ATTRIBUTE(qreal, keyMarginTop, KeyMarginTop)
    M_STYLE_ATTRIBUTE(qreal, keyMarginRight, KeyMarginRight)
    M_STYLE_ATTRIBUTE(qreal, keyMarginBottom, KeyMarginBottom)

    M_STYLE_ATTRIBUTE(bool, syncStyleModeWithKeyCount, SyncStyleModeWithKeyCount)
};

class M_EXPORT MImAbstractKeyAreaStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(MImAbstractKeyAreaStyle)
    M_STYLE_MODE(Keys13)
    M_STYLE_MODE(Keys30)
    M_STYLE_MODE(Keys31)
    M_STYLE_MODE(Keys32)
    M_STYLE_MODE(Keys33)
    M_STYLE_MODE(Keys34)
    M_STYLE_MODE(Keys35)
    M_STYLE_MODE(Keys36)
    M_STYLE_MODE(Keys37)
    M_STYLE_MODE(Keys38)
    M_STYLE_MODE(Keys39)
    M_STYLE_MODE(Keys40)
    M_STYLE_MODE(Keys41)
    M_STYLE_MODE(Keys42)
    M_STYLE_MODE(Keys43)
    M_STYLE_MODE(Keys44)
    M_STYLE_MODE(Keys45)
};


#endif // MIMABSTRACTKEYAREASTYLE_H
