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

#include <QDebug>
#include "mxkb.h"
#include "mxkb_p.h"
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

MXkbPrivate::MXkbPrivate()
    : display(0),
      deviceSpec(0),
      qtAltMask(0),
      qtMetaMask(0),
      qtModeSwitchMask(0)

{
    init();
}

MXkbPrivate::~MXkbPrivate()
{
}

void MXkbPrivate::init()
{
    char *displayName = static_cast<char *>(getenv("DISPLAY"));

    int event, error, status;
    display = XkbOpenDisplay(displayName, &event, &error, NULL, NULL, &status);
    if (!display) {
        qFatal("%s xkb open the display error!", __PRETTY_FUNCTION__);
        return;
    }

    int opcode = -1;
    int xkbEventBase = -1;
    int xkbErrorBase = -1;
    int xkblibMajor = XkbMajorVersion;
    int xkblibMinor = XkbMinorVersion;
    if (!XkbLibraryVersion(&xkblibMajor, &xkblibMinor)) {
        qFatal("%s xkb query version error!", __PRETTY_FUNCTION__);
        return;
    }
    if (!XkbQueryExtension(display, &opcode, &xkbEventBase, &xkbErrorBase, &xkblibMajor, &xkblibMinor)) {
        qFatal("%s xkb query extension error!", __PRETTY_FUNCTION__);
        return;
    }

    XSynchronize(display, 1);
    //TODO:XkbUseCoreKbd may change in the device and it should be queried from xkb.
    deviceSpec = XkbUseCoreKbd;
    getMask();
}

unsigned int MXkbPrivate::translateModifiers(Qt::KeyboardModifiers modifier) const
{
    unsigned int ret = 0;
    if (modifier & Qt::ShiftModifier)
        ret |= ShiftMask;
    if (modifier & Qt::AltModifier)
        ret |= qtAltMask;
    if (modifier & Qt::MetaModifier)
        ret |= qtMetaMask;
    if (modifier & Qt::GroupSwitchModifier)
        ret |= qtModeSwitchMask;
    return ret;
}

void MXkbPrivate::getMask()
{
    //TODO:The xkb keymap could be changed at anytime when user chooses a new keymap in the MSettings.
    //But the Gconf which will be used to notify reloading keymap is not yet defined.
    XkbDescPtr xkbDesc = XkbGetMap(display, XkbAllClientInfoMask, deviceSpec);
    for (int i = xkbDesc->min_key_code; i < xkbDesc->max_key_code; ++i) {
        const unsigned int mask = xkbDesc->map->modmap ? xkbDesc->map->modmap[i] : 0;
        if (mask == 0) {
            // key is not bound to a modifier
            continue;
        }
        for (int j = 0; j < XkbKeyGroupsWidth(xkbDesc, i); ++j) {
            const KeySym keySym = XkbKeySym(xkbDesc, i, j);
            if (keySym == NoSymbol)
                continue;
            setMask(keySym, mask);
        }
    }
    XkbFreeKeyboard(xkbDesc, XkbAllComponentsMask, true);
}

inline void MXkbPrivate::setMask(const KeySym &sym, unsigned int mask)
{
    if (qtAltMask == 0
            && qtMetaMask != mask
            && (sym == XK_Alt_L || sym == XK_Alt_R)) {
        qtAltMask = mask;
    }
    if (qtMetaMask == 0
            && qtAltMask != mask
            && (sym == XK_Meta_L || sym == XK_Meta_R)) {
        qtMetaMask = mask;
    }
    if (qtModeSwitchMask == 0
            && qtAltMask != mask
            && qtMetaMask != mask
            && sym == XK_Mode_switch) {
        qtModeSwitchMask = mask;
    }
}

inline bool MXkbPrivate::testModifierLatchedState(int xModifier) const
{
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    unsigned int mask;
    XQueryPointer(display, DefaultRootWindow(display), &dummy1, &dummy2,
                  &dummy3, &dummy4, &dummy5, &dummy6, &mask);
    return (mask & xModifier);
}

MXkb::MXkb()
    : d_ptr(new MXkbPrivate())
{
}

MXkb::~MXkb()
{
    Q_D(MXkb);
    delete d;
}

void MXkb::lockModifiers(Qt::KeyboardModifiers modifiers)
{
    Q_D(MXkb);
    //FIXME: a tricky case, have to call both XkbLatchModifiers and XkbLockModifiers here,
    //otherwise can not lock the modifiers.
    const unsigned int xModifiers = d->translateModifiers(modifiers);
    if (!XkbLatchModifiers(d->display, d->deviceSpec, xModifiers, xModifiers))
        qWarning() << __PRETTY_FUNCTION__ << " failed!";
    if (!XkbLockModifiers(d->display, d->deviceSpec, xModifiers, xModifiers))
        qWarning() << __PRETTY_FUNCTION__ << " failed!";
}

void MXkb::unlockModifiers(Qt::KeyboardModifiers modifiers)
{
    Q_D(MXkb);
    const unsigned int xModifiers = d->translateModifiers(modifiers);
    if (!XkbLockModifiers(d->display, d->deviceSpec, xModifiers, 0))
        qWarning() << __PRETTY_FUNCTION__ << " failed!";
    if (!XkbLatchModifiers(d->display, d->deviceSpec, xModifiers, 0))
        qWarning() << __PRETTY_FUNCTION__ << " failed!";
}

bool MXkb::isLatched(Qt::KeyboardModifier modifier) const
{
    Q_D(const MXkb);
    int xModifier = d->translateModifiers(modifier);
    return d->testModifierLatchedState(xModifier);
}
