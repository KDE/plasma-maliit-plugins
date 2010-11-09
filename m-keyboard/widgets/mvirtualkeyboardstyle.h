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



#ifndef MVIRTUALKEYBOARDSTYLE_H
#define MVIRTUALKEYBOARDSTYLE_H

#include <mwidgetstyle.h>
#include <QFont>

class MVirtualKeyboard;
class MScalableImage;

/*!
 * \class MVirtualKeyboardStyle
 * \brief This class provides access to style attributes for MVirtualKeyboard
 */
class MVirtualKeyboardStyle : public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(MVirtualKeyboardStyle)

public:
    M_STYLE_ATTRIBUTE(QSize, keyboardAreaSize, KeyboardAreaSize)

    M_STYLE_ATTRIBUTE(qreal, fontOpacity, FontOpacity)

    M_STYLE_ATTRIBUTE(QSize, tabButtonSize, TabButtonSize)

    M_STYLE_ATTRIBUTE(QSize, menuSize, MenuSize)

    M_STYLE_ATTRIBUTE(QFont, notificationFont, NotificationFont)
    M_STYLE_ATTRIBUTE(int, notificationFontSize, NotificationFontSize)
    M_STYLE_ATTRIBUTE(QColor, notificationBorderColor, NotificationBorderColor)
    M_STYLE_ATTRIBUTE(QColor, notificationBackgroundColor, NotificationBackgroundColor)
    M_STYLE_ATTRIBUTE(QColor, notificationTextColor, NotificationTextColor)
    M_STYLE_ATTRIBUTE(qreal, notificationOpacity, NotificationOpacity)

    M_STYLE_ATTRIBUTE(QColor, deadkeyLockedColor, DeadkeyLockedColor)
};

class MVirtualKeyboardStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(MVirtualKeyboardStyle)

    M_STYLE_MODE(Landscape)
    M_STYLE_MODE(Portrait)

    friend class MVirtualKeyboard;
};

#endif
