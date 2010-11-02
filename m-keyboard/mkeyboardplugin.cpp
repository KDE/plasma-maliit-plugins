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



#include <MLocale>
#include "mkeyboardplugin.h"
#include "mkeyboardhost.h"
#include "mkeyboardsettings.h"

#include <QtPlugin>

#include <mtimestamp.h>

MKeyboardPlugin::MKeyboardPlugin()
    : translationIsLoaded(false)
{
}

QString MKeyboardPlugin::name() const
{
    return "MeegoKeyboard";
}


QStringList MKeyboardPlugin::languages() const
{
    return QStringList("en");
}


MAbstractInputMethod *
MKeyboardPlugin::createInputMethod(MAbstractInputMethodHost *host)
{
    loadTranslation();
    mTimestamp("MKeyboardPlugin", "start");
    MAbstractInputMethod *inputMethod = new MKeyboardHost(host);
    mTimestamp("MKeyboardPlugin", "end");
    return inputMethod;
}

MInputMethodSettingsBase *MKeyboardPlugin::createInputMethodSettings()
{
    loadTranslation();
    MInputMethodSettingsBase *inputMethodSettings = new MKeyboardSettings;
    return inputMethodSettings;
}

QSet<MInputMethod::HandlerState> MKeyboardPlugin::supportedStates() const
{
    QSet<MInputMethod::HandlerState> result;

    result << MInputMethod::OnScreen << MInputMethod::Hardware;
    return result;
}

void MKeyboardPlugin::loadTranslation()
{
    if (!translationIsLoaded) {
        MLocale locale;
        // add virtual-keyboard and hardware-keyboard catalog
        locale.installTrCatalog("virtual-keyboard");
        locale.installTrCatalog("hardware-keyboard");
        MLocale::setDefault(locale);
        translationIsLoaded = true;
    }
}

Q_EXPORT_PLUGIN2(meego-keyboard, MKeyboardPlugin)
