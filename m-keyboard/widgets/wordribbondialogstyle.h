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


#ifndef WORDRIBBONDIALOGSTYLE_H
#define WORDRIBBONDIALOGSTYLE_H

#include <mdialogstyle.h>

class WordRibbonDialogStyle: public MDialogStyle
{
    Q_OBJECT
    M_STYLE(WordRibbonDialogStyle)

};

class M_EXPORT WordRibbonDialogStyleContainer : public MDialogStyleContainer
{
    M_STYLE_CONTAINER(WordRibbonDialogStyle)
    friend class WordRibbonDialog;
};

#endif // WORDRIBBONDIALOGSTYLE_H
