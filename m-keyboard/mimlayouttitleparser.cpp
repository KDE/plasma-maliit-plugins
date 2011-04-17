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

#include "mimlayouttitleparser.h"

#include <QDebug>

MImLayoutTitleParser::MImLayoutTitleParser(QIODevice *device)
    : xml(device),
      mTitle()
{
}

bool MImLayoutTitleParser::parse()
{
    findRootElement();

    if (!xml.hasError())
        parseKeyboard();

    return !xml.hasError();
}

void MImLayoutTitleParser::findRootElement()
{
    Q_ASSERT(xml.tokenType() == QXmlStreamReader::NoToken);

    while (!xml.atEnd()) {
        QXmlStreamReader::TokenType type = xml.readNext();
        if (type == QXmlStreamReader::StartElement)
            return;
    }

    Q_ASSERT(xml.hasError());
}

void MImLayoutTitleParser::error(const QString &message)
{
    if (xml.hasError())
        return;

    xml.raiseError(message);
}

void MImLayoutTitleParser::parseKeyboard()
{
    Q_ASSERT(xml.isStartElement());

    if (!xml.isStartElement() || xml.name() != QLatin1String("keyboard")) {
        error(QString::fromLatin1("Expected '<keyboard>', but got '<%1>'.").arg(xml.name().toString()));
    }

    const QXmlStreamAttributes& attributes = xml.attributes();
    mTitle = attributes.value(QLatin1String("title")).toString();
}

const QString MImLayoutTitleParser::errorString() const
{
    return xml.errorString();
}

const QString MImLayoutTitleParser::keyboardTitle() const
{
    return mTitle;
}
