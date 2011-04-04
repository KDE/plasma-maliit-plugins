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


#ifndef WORDRIBBONITEMSTYLE_H
#define WORDRIBBONITEMSTYLE_H

#include <mwidgetstyle.h>

class WordRibbonItemStyle: public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(WordRibbonItemStyle)

public:
    M_STYLE_ATTRIBUTE(QFont,  font, Font)
    M_STYLE_ATTRIBUTE(QColor, fontColor, FontColor)
};

class WordRibbonItemStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(WordRibbonItemStyle)

    M_STYLE_MODE(Pressed)
    M_STYLE_MODE(Selected)
    M_STYLE_MODE(Onecharacter)
    M_STYLE_MODE(Twocharacter)
    M_STYLE_MODE(Morecharacter)

    friend class WordRibbonItem;
};

#endif // WORDRIBBONITEMSTYLE_H

