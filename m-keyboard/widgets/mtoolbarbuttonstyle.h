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

#ifndef TOOLBAR_BUTTON_H
#define TOOLBAR_BUTTON_H

#include <mbuttoniconstyle.h>

// Inheriting from MWidgetStyle crashes when it tries to set the button's label font!
class M_EXPORT MToolbarButtonStyle : public MButtonIconStyle
{
    Q_OBJECT
    M_STYLE(MToolbarButtonStyle)
};

class M_EXPORT MToolbarButtonStyleContainer : public MButtonIconStyleContainer
{
    M_STYLE_CONTAINER(MToolbarButtonStyle)
};

#endif

