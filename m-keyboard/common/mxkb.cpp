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

#include <QX11Info>
#include <QtDebug>
#include "mxkb.h"
#include <X11/XKBlib.h>

MXkb::MXkb()
    : deviceSpec(0)
{
    int opcode = -1;
    int xkbEventBase = -1;
    int xkbErrorBase = -1;
    int xkblibMajor = XkbMajorVersion;
    int xkblibMinor = XkbMinorVersion;
    if (!XkbLibraryVersion(&xkblibMajor, &xkblibMinor)) {
        qFatal("%s xkb query version error!", __PRETTY_FUNCTION__);
        return;
    }

    Display* display = QX11Info::display();
    if (!XkbQueryExtension(display, &opcode, &xkbEventBase, &xkbErrorBase, &xkblibMajor, &xkblibMinor)) {
        qFatal("%s xkb query extension error!", __PRETTY_FUNCTION__);
        return;
    }

    // TODO: XkbUseCoreKbd may change in the device and it should be queried from xkb.
    deviceSpec = XkbUseCoreKbd;
}

MXkb::~MXkb()
{
}

void MXkb::lockModifiers(unsigned int affect, unsigned int values)
{
    if (!XkbLockModifiers(QX11Info::display(), deviceSpec, affect, values))
        qWarning() << __PRETTY_FUNCTION__ << " failed!";
}

void MXkb::latchModifiers(unsigned int affect, unsigned int values)
{
    if (!XkbLatchModifiers(QX11Info::display(), deviceSpec, affect, values))
        qWarning() << __PRETTY_FUNCTION__ << " failed!";
}
