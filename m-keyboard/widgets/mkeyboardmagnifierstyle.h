/* * This file is part of dui-vkb-magnifier *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: Tomas Junnonen <tomas.junnonen@nokia.com>
 *
 * This software, including documentation, is protected by copyright
 * controlled by Nokia Corporation. All rights are reserved. Copying,
 * including reproducing, storing, adapting or translating, any or all of
 * this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 */

#ifndef MKEYBOARDMAGNIFIERSTYLE_H
#define MKEYBOARDMAGNIFIERSTYLE_H

#include <MWidgetStyle>
#include <QFont>

class MScalableImage;

/*!
 * \class MKeyboardMagnifierStyle
 * \brief This class provides access to style attributes for virtual keyboard magnifier
 */
class MKeyboardMagnifierStyle: public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(MKeyboardMagnifierStyle)

public:
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, magnifierImage, magnifierImage)
    M_STYLE_ATTRIBUTE(QSize, magnifierSize, magnifierSize)
    M_STYLE_ATTRIBUTE(int, magnifierArrowHeight, MagnifierArrowHeight)
    M_STYLE_ATTRIBUTE(int, magnifierButtonOverlap, MagnifierButtonOverlap)
    M_STYLE_ATTRIBUTE(QFont, magnifierFont, MagnifierFont)
    M_STYLE_ATTRIBUTE(int, magnifierFontSize, MagnifierFontSize)
    M_STYLE_ATTRIBUTE(QColor, magnifierTextColor, MagnifierTextColor)
    M_STYLE_ATTRIBUTE(int, magnifierTextMarginTop, MagnifierTextMarginTop)
    M_STYLE_ATTRIBUTE(int, magnifierTextMarginBottom, MagnifierTextMarginBottom)
    M_STYLE_ATTRIBUTE(int, magnifierHideDelay, MagnifierHideDelay)

    M_STYLE_ATTRIBUTE(int, extendedKeysOffset, ExtendedKeysOffset)
};

class MKeyboardMagnifierStyleContainer: public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(MKeyboardMagnifierStyle)

    M_STYLE_MODE(Landscape)
    M_STYLE_MODE(Portrait)
};

#endif
