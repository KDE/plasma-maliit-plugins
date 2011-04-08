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



#include "hwkbcharloopsmanager.h"
#include "hwkbcharloops.h"
#include <algorithm>
#include <QDomDocument>
#include <QFile>
#include <QDebug>
#include <QStringList>

namespace
{
    const QString SystemDisplayLanguage("/meegotouch/i18n/language");
    const QString HardwareKeyboardCharLoopsFile("hwkbcharloops.xml");
    const QString HwKbCharLoopConfigurationPath = "/usr/share/meegotouch/virtual-keyboard/layouts/";
    const QString DefaultDisplayLanguage("en_gb");

    const QString HWKBTagKeyboard           = QString("keyboard");
    const QString HWKBTagVersion            = QString("version");
    const QString HWKBTagLanguage           = QString("language");
    const QString HWKBTagLanguageCode       = QString("language_code");
    const QString HWKBTagName               = QString("name");
    const QString HWKBTagCharacter          = QString("character");
    const QString HWKBLabel                 = QString("label");
    const QString HWKBAccentedLabel         = QString("accented_labels");
};

struct HwKbCharLoopParseParameters {
    //! Contains true if current XML tag was successfully parsed
    bool validTag;

    HwKbCharacterLoops *currentCharLoop;

    const QString *fileName;

    HwKbCharLoopParseParameters();
};

HwKbCharLoopParseParameters::HwKbCharLoopParseParameters()
    : validTag(true),
      currentCharLoop(0),
      fileName(0)
{
}

HwKbCharLoopsManager::HwKbCharLoopsManager()
    : current(0),
      configLanguage(SystemDisplayLanguage)
{
    loadCharLoops(HardwareKeyboardCharLoopsFile);
    // Read settings for the first time to set active character loop
    syncLanguage();

    // Synchronize with settings when someone changes them (e.g. via control panel).
    connect(&configLanguage, SIGNAL(valueChanged()), this, SLOT(syncLanguage()));
}

HwKbCharLoopsManager::~HwKbCharLoopsManager()
{
    qDeleteAll(charLoops);
    charLoops.clear();
}

bool HwKbCharLoopsManager::setCharLoopsLanguage(const QString &language)
{
    bool val = true;
    const QStringList icuLocaleParts(language.split("_", QString::SkipEmptyParts));

    if (currentCharLoopLanguage != language) {
        if (charLoops.contains(language)) {
            current = charLoops.constFind(language).value();
        } else if (!icuLocaleParts.isEmpty() && charLoops.contains(icuLocaleParts[0])) {
            current = charLoops.constFind(icuLocaleParts[0]).value();
        } else {
            current = 0;
            val = false;
        }
        currentCharLoopLanguage = language;
    }
    return val;
}

void HwKbCharLoopsManager::syncLanguage()
{
    QString language(configLanguage.value().toString());
    if (language.isEmpty()) {
        language = DefaultDisplayLanguage;
    }
    setCharLoopsLanguage(language);
}

QString HwKbCharLoopsManager::characterLoop(const QChar &c) const
{
    QString charLoop;
    if (current != 0) {
        charLoop = current->loops.value(c);
    }
    return charLoop;
}

bool HwKbCharLoopsManager::loadCharLoops(const QString &fileName)
{
    qDebug() << __PRETTY_FUNCTION__ << fileName;
    HwKbCharLoopParseParameters params;
    qDeleteAll(charLoops);
    charLoops.clear();

    const bool success = loadCharLoopsImpl(fileName, params);
    if (!success) {
        qDeleteAll(charLoops);
        charLoops.clear();
    }
    current = 0;
    return success;
}

bool HwKbCharLoopsManager::loadCharLoopsImpl(const QString &fileName, HwKbCharLoopParseParameters &params)
{

    const QString absoluteFileName = HwKbCharLoopConfigurationPath + fileName;

    params.fileName = &absoluteFileName;
    if (!QFile::exists(absoluteFileName)) {
        qWarning() << "HwKb character loops file" << absoluteFileName << "does not exist.";
        return false;
    }

    QFile infile(absoluteFileName);
    QString errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument doc;

    if (!infile.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open HwKb character loops file" << absoluteFileName;
        return false;
    }
    if (!doc.setContent((QIODevice *)(&infile), true, &errorStr, &errorLine,
                        &errorColumn)) {
        qWarning() << "Invalid HwKb character loops file" << absoluteFileName;
        qWarning("Parse error on line %d column %d: %s", errorLine, errorColumn,
                 qPrintable(errorStr));
        return false;
    }

    bool valid = true;
    const QDomElement root = doc.documentElement();

    //check the root tag
    if (!root.isNull() && root.tagName() != HWKBTagKeyboard) {
        qWarning() << "Invalid virtual keyboard layout file" << absoluteFileName;
        valid = false;
    } else {
        keyboardVersion = root.attribute(HWKBTagVersion);

        parseChildren(root, params, &HWKBTagLanguage, &HwKbCharLoopsManager::parseTagLanguage);
        valid = params.validTag;
    }

    return valid;

}

void HwKbCharLoopsManager::parseChildren(const QDomElement &element, HwKbCharLoopParseParameters &params,
                                         const QString *tag1, TagParser parser1)
{
    Q_ASSERT(tag1);
    Q_ASSERT(parser1);

    for (QDomNode child = element.firstChild(); !child.isNull() && params.validTag;
            child = child.nextSibling()) {
        if (child.isElement()) {
            const QDomElement childElement = child.toElement();
            if (childElement.tagName() == *tag1) {
                (this->*parser1)(childElement, params);
            } else {
                qWarning() << "Unexpected tag" << childElement.tagName() << "on line"
                           << childElement.lineNumber() << "column" << childElement.columnNumber()
                           << "in character loop file" << *params.fileName;
                qWarning() << "The only allowed tag is" << *tag1;
                params.validTag = false;
            }
        }
    }
}

void HwKbCharLoopsManager::parseTagLanguage(const QDomElement &element, HwKbCharLoopParseParameters &params)
{
    params.currentCharLoop = 0;
    const QString language = element.attribute(HWKBTagLanguageCode);
    const QString name = element.attribute(HWKBTagName);
    if (charLoops.contains(language)) {
        //if already contains, overwrite.
        params.currentCharLoop = charLoops[language];
        params.currentCharLoop->loops.clear();
    } else {
        params.currentCharLoop = new HwKbCharacterLoops(language, name);
        charLoops.insert(language, params.currentCharLoop);
    }

    parseChildren(element, params, &HWKBTagCharacter, &HwKbCharLoopsManager::parseTagCharacter);
}

void HwKbCharLoopsManager::parseTagCharacter(const QDomElement &element, HwKbCharLoopParseParameters &params)
{
    const QString character = element.attribute(HWKBLabel);
    const QString accentedCharacters = element.attribute(HWKBAccentedLabel);
    if ((character.length() == 1) && !accentedCharacters.isEmpty()) {
        params.currentCharLoop->loops.insert(character.at(0), accentedCharacters);
    }
}

