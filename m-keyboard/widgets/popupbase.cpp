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



#include "popupbase.h"
#include "mvirtualkeyboardstyle.h"

#include <QDebug>
#include <MTheme>

PopupBase::PopupBase(const MVirtualKeyboardStyleContainer &styleContainer)
    // TODO: Even though style value caching is apparently discouraged, we
    // have to cache these pixmaps for now since popup size and
    // boundingRect() result is (incorrectly) based on their size.  Presumably
    // this caching should be eliminated when the popup is updated to
    // match the layout guide.
    : styleContainer(styleContainer)
{
}

PopupBase::~PopupBase()
{
}


void PopupBase::setTargetButton(const IKeyButton *key)
{
    Q_UNUSED(key)
}

