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

#ifndef HWKBCHARLOOPSMANAGER_STUB_H
#define HWKBCHARLOOPSMANAGER_STUB_H

#include "mgconfitem_stub.h"
#include "hwkbcharloopsmanager.h"
#include <QDebug>

namespace
{
    const QString SystemDisplayLanguage("/M/i18n/Language");
    const QString StubAccentedCharacters = QString("%1%2%3%4%5%6")
                                           .arg(QChar(0x00E4))
                                           .arg(QChar(0x00E0))
                                           .arg(QChar(0x00E2))
                                           .arg(QChar(0x00E1))
                                           .arg(QChar(0x00E3))
                                           .arg(QChar(0x00E5));
};

HwKbCharLoopsManager::HwKbCharLoopsManager()
    : configLanguage(SystemDisplayLanguage)
{
}

HwKbCharLoopsManager::~HwKbCharLoopsManager()
{
}

QString HwKbCharLoopsManager::characterLoop(const QChar &c) const
{
    // Return empty text for 'b' character for one test case.
    if (c == QChar('b'))
        return QString();
    else
        return StubAccentedCharacters;
}

#endif
