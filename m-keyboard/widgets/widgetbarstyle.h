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



#ifndef WIDGETBARSTYLE_H
#define WIDGETBARSTYLE_H

#include <MWidgetStyle>

/*!
    \brief This defines style that is common to all WidgetBar class based widgets.
           Currently such classes are tab buttons of Sym view and toolbar of the keyboard.
*/
class M_EXPORT WidgetBarStyle : public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(WidgetBarStyle)
};

class M_EXPORT WidgetBarStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(WidgetBarStyle)
};


#endif
