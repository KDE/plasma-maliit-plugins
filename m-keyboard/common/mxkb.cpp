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
#include <QString>
#include <QtDebug>
#include "mxkb.h"
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

namespace {
    const QString XkbDataDir("/usr/share/X11/xkb/rules");
};

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

bool MXkb::setXkbMap(const QString &modelName, const QString &layoutName, const QString &variantName)
{
    qDebug() << __PRETTY_FUNCTION__;
    bool val = false;
    if (modelName.isEmpty() || layoutName.isEmpty())
        return val;

    Display *display = QX11Info::display();
    QByteArray buffer;
    char *rulesName = NULL;
    XkbRF_RulesPtr rules = NULL;
    XkbRF_VarDefsRec defs_rec;
    XkbComponentNamesRec names_rec;
    XkbDescPtr xkb = NULL;
    char *locale = setlocale(LC_ALL, NULL);

    QByteArray model = modelName.toAscii();
    QByteArray layout = layoutName.toAscii();
    QByteArray variant = variantName.toAscii();

    XkbRF_GetNamesProp(display, &rulesName, &defs_rec);
    if (rulesName == NULL) {
        qWarning("Couldn't get rules' name");
        goto exit_door;
    }

    buffer = QString(XkbDataDir + "/" + rulesName).toAscii();

    defs_rec.model  = model.data();
    defs_rec.layout = layout.data();
    if (!variantName.isEmpty()) {
        defs_rec.variant = variant.data();
    } else {
        defs_rec.variant = 0;
    }
    defs_rec.options = 0;

    rules = XkbRF_Load(buffer.data(), locale, true, true);
    if (rules == NULL)  {
        qWarning() << "Couldn't get rules";
        goto exit_door;
    }

    if (!XkbRF_GetComponents(rules, &defs_rec, &names_rec)) {
        qWarning() << "Couldn't get components";
        goto exit_door;
    }

    xkb = XkbGetKeyboardByName(display, deviceSpec,
                               &names_rec, XkbGBN_AllComponentsMask,
                               XkbGBN_AllComponentsMask&(~XkbGBN_GeometryMask),
                               true);
    if (!xkb) {
        qCritical("Error loading new keyboard description");
        goto exit_door;
    }

    if (!XkbRF_SetNamesProp(display, rulesName, &defs_rec)) {
        qWarning() << "Error updating XKB names property";
        goto exit_door;
    }

    val = true;
exit_door:
    if (rulesName) {
        free(rulesName);
        rulesName = NULL;
    }
    if (rules) {
        XkbRF_Free(rules, true);
    }
    if (xkb) {
        XkbFreeKeyboard(xkb, XkbAllComponentsMask, true);
    }
    return val;
}
