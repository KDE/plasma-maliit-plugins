/* * This file is part of dui-keyboard *
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



#ifndef DUIVIRTUALKEYBOARDSTYLE_H
#define DUIVIRTUALKEYBOARDSTYLE_H

#include <duiwidgetstyle.h>
#include <QFont>

class DuiVirtualKeyboard;
class DuiScalableImage;

/*!
 * \class DuiVirtualKeyboardStyle
 * \brief This class provides access to style attributes for DuiVirtualKeyboard
 */
class DuiVirtualKeyboardStyle : public DuiWidgetStyle
{
    Q_OBJECT
    DUI_STYLE(DuiVirtualKeyboardStyle)

public:
    DUI_STYLE_ATTRIBUTE(QSize, keyboardAreaSize, KeyboardAreaSize)
    DUI_STYLE_ATTRIBUTE(int, spacingVertical, SpacingVertical)
    DUI_STYLE_ATTRIBUTE(int, spacingHorizontal, SpacingHorizontal)
    DUI_STYLE_ATTRIBUTE(QColor, fontColor, FontColor)
    DUI_STYLE_ATTRIBUTE(qreal, fontOpacity, FontOpacity)
    DUI_STYLE_ATTRIBUTE(QFont, font, Font)
    DUI_STYLE_ATTRIBUTE(int, labelMarginTop, LabelMarginTop)
    DUI_STYLE_ATTRIBUTE(int, labelMarginBottom, LabelMarginBottom)
    DUI_STYLE_ATTRIBUTE(QSize, tabButtonSize, TabButtonSize)

    DUI_STYLE_ATTRIBUTE(QSize, menuSize, MenuSize)

    DUI_STYLE_ATTRIBUTE(QFont, notificationFont, NotificationFont)
    DUI_STYLE_ATTRIBUTE(QColor, notificationBorderColor, NotificationBorderColor)
    DUI_STYLE_ATTRIBUTE(QColor, notificationBackgroundColor, NotificationBackgroundColor)
    DUI_STYLE_ATTRIBUTE(QColor, notificationTextColor, NotificationTextColor)
    DUI_STYLE_ATTRIBUTE(qreal, notificationOpacity, NotificationOpacity)

    DUI_STYLE_PTR_ATTRIBUTE(DuiScalableImage *, toolbarBackgroundImage, ToolbarBackgroundImage);

    DUI_STYLE_ATTRIBUTE(QColor, deadkeyLockedColor, DeadkeyLockedColor)

    DUI_STYLE_ATTRIBUTE(QSize, keyNormalSize, KeyNormalSize)
    DUI_STYLE_ATTRIBUTE(QSize, keyFunctionNormalSize, KeyFunctionNormalSize)
    DUI_STYLE_ATTRIBUTE(QSize, keyFunctionLargeSize, KeyFunctionLargeSize)
    DUI_STYLE_ATTRIBUTE(QSize, keyPhoneNumberNormalSize, KeyPhoneNumberNormalSize)
    DUI_STYLE_ATTRIBUTE(QSize, keyNumberBackspaceSize, KeyNumberBackspaceSize)
    DUI_STYLE_ATTRIBUTE(QSize, keySymNormalSize, KeySymNormalSize)

    DUI_STYLE_ATTRIBUTE(QString, keyBackgroundId, KeyBackgroundId)
    DUI_STYLE_ATTRIBUTE(QString, keyBackgroundPressedId, KeyBackgroundPressedId)
    DUI_STYLE_ATTRIBUTE(QString, keyBackgroundSelectedId, KeyBackgroundSelectedId)

    DUI_STYLE_PTR_ATTRIBUTE(DuiScalableImage *, keyBackgroundSymIndicatorSym, KeyBackgroundSymIndicatorSym)
    DUI_STYLE_PTR_ATTRIBUTE(DuiScalableImage *, keyBackgroundSymIndicatorAce, KeyBackgroundSymIndicatorAce)

    DUI_STYLE_ATTRIBUTE(QSize, keyBackspaceIconSize, KeyBackspaceIconSize)
    DUI_STYLE_ATTRIBUTE(QString, keyBackspaceIconId, KeyBackspaceIconId)

    DUI_STYLE_ATTRIBUTE(QSize, keyMenuIconSize, KeyMenuIconSize)
    DUI_STYLE_ATTRIBUTE(QString, keyMenuIconId, KeyMenuIconId)

    DUI_STYLE_ATTRIBUTE(QSize, keyEnterIconSize, KeyEnterIconSize)
    DUI_STYLE_ATTRIBUTE(QString, keyEnterIconId, KeyEnterIconId)

    DUI_STYLE_ATTRIBUTE(QSize, keyShiftIconSize, KeyShiftIconSize)
    DUI_STYLE_ATTRIBUTE(QString, keyShiftIconId, KeyShiftIconId)
};

class DuiVirtualKeyboardStyleContainer : public DuiWidgetStyleContainer
{
    DUI_STYLE_CONTAINER(DuiVirtualKeyboardStyle)

    DUI_STYLE_MODE(Landscape)
    DUI_STYLE_MODE(Portrait)

    friend class DuiVirtualKeyboard;
};

#endif
