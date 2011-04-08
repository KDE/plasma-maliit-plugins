/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
