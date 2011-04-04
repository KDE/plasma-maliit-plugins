/* * This file is part of meego-keyboard-zh *
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


#ifndef WORDRIBBONSTYLE_H
#define WORDRIBBONSTYLE_H

#include <mwidgetstyle.h>
#include <QDebug>

class WordRibbonStyle: public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(WordRibbonStyle)
public:
    M_STYLE_ATTRIBUTE(int,  spaceBetween, SpaceBetween)
};

class WordRibbonStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(WordRibbonStyle)

    M_STYLE_MODE(Landscape)
    M_STYLE_MODE(Portrait)
    M_STYLE_MODE(Dialogmode)

    friend class WordRibbon;
};

#endif // WORDRIBBONSTYLE_H
