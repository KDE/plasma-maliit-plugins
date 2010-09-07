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

#ifndef HWKBCHARLOOPSMANAGER_STUB_H
#define HWKBCHARLOOPSMANAGER_STUB_H

#include "mgconfitem_stub.h"
#include "hwkbcharloopsmanager.h"
#include <QDebug>

namespace
{
    const QString SystemDisplayLanguage("/meegotouch/i18n/Language");
    const QString StubAccentedCharactersCapitalA = QString("%1%2%3%4%5%6")
        .arg(QChar(0x00C4))
        .arg(QChar(0x00C0))
        .arg(QChar(0x00C2))
        .arg(QChar(0x00C1))
        .arg(QChar(0x00C3))
        .arg(QChar(0x00C5));
    const QString StubAccentedCharactersA = QString("%1%2%3%4%5%6")
        .arg(QChar(0x00E4))
        .arg(QChar(0x00E0))
        .arg(QChar(0x00E2))
        .arg(QChar(0x00E1))
        .arg(QChar(0x00E3))
        .arg(QChar(0x00E5));
    const QString StubAccentedCharactersO = QString("%1%2%3%4%5%6")
        .arg(QChar(0x00F6))
        .arg(QChar(0x00F2))
        .arg(QChar(0x00F3))
        .arg(QChar(0x00F4))
        .arg(QChar(0x00F5))
        .arg(QChar(0x00F8));
};

HwKbCharLoopsManager::HwKbCharLoopsManager()
    : current(0),
      configLanguage(SystemDisplayLanguage)
{
}

HwKbCharLoopsManager::~HwKbCharLoopsManager()
{
}

QString HwKbCharLoopsManager::characterLoop(const QChar &c) const
{
    if (c == QChar('a')) {
        return StubAccentedCharactersA;
    } else if (c == QChar('A')) {
        return StubAccentedCharactersCapitalA;
    } else if (c == QChar('o')) {
        return StubAccentedCharactersO;
    } else {
        return QString();
    }
}

#endif
