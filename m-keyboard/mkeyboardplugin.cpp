/* * This file is part of m-keyboard *
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



#include "mkeyboardplugin.h"
#include "mkeyboardhost.h"

#include <QtPlugin>

#include <mtimestamp.h>


QString MKeyboardPlugin::name() const
{
    return "MeegoKeyboard";
}


QStringList MKeyboardPlugin::languages() const
{
    return QStringList("en");
}


MInputMethodBase *
MKeyboardPlugin::createInputMethod(MInputContextConnection *icConnection)
{
    mTimestamp("MKeyboardPlugin", "start");
    MInputMethodBase *inputMethod = new MKeyboardHost(icConnection);
    mTimestamp("MKeyboardPlugin", "end");
    return inputMethod;
}


Q_EXPORT_PLUGIN2(mvirtualkeyboard, MKeyboardPlugin)

