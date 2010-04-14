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



#include "hwkbcharloopsmanager.h"
#include "hwkbcharloops.h"
#include <algorithm>
#include <QDomDocument>
#include <QFile>
#include <QDebug>

namespace
{
    const QString SystemDisplayLanguage("/meegotouch/i18n/language");
    const QString HardwareKeyboardCharLoopsFile("hwkbcharloops.xml");
    const QString HwKbCharLoopConfigurationPath = "/usr/share/meegotouch/virtual-keyboard/layouts/";

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
    : configLanguage(SystemDisplayLanguage)
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
    qDebug() << __PRETTY_FUNCTION__ << language;
    bool val = true;
    if (currentCharLoopLanguage != language) {
        if (charLoops.contains(language)) {
            current = charLoops.constFind(language);
        } else {
            current = charLoops.constEnd();
            val = false;
        }
        currentCharLoopLanguage = language;
    }
    return val;
}

void HwKbCharLoopsManager::syncLanguage()
{
    if (!configLanguage.value().isNull()) {
        setCharLoopsLanguage(configLanguage.value().toString());
    }
}

QString HwKbCharLoopsManager::characterLoop(const QChar &c) const
{
    QString charLoop;
    if (current != charLoops.constEnd()) {
        charLoop = current.value()->loops.value(c);
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
        params.currentCharLoop->loops.clear();
        params.currentCharLoop = charLoops[language];
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

