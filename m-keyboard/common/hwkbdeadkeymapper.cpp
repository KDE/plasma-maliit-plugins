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

#include "hwkbdeadkeymapper.h"
#include <algorithm>
#include <QDomDocument>
#include <QFile>
#include <QDebug>

namespace
{
    const char * const XkbLayoutSettingName("/meegotouch/inputmethods/hwkeyboard/layout");
    const char * const XkbVariantSettingName("/meegotouch/inputmethods/hwkeyboard/variant");

    const char * const DeadKeyMapFile("hwkb_dead_keys.xml");
    const char * const DeadKeyMapDirectory("/usr/share/meegotouch/virtual-keyboard/layouts/");

    const char * const HWKBTagMappings("mappings");
    const char * const HWKBTagVersion("version");
    const char * const HWKBTagLanguage("language");
    const char * const HWKBTagInclude("include");
    const char * const HWKBTagMapping("mapping");
    const char * const HWKBTagLayout("layout");
    const char * const HWKBTagVariant("variant");
    const char * const HWKBTagResult("result");
    const char * const HWKBTagFrom("from");
};

struct HwKbDeadKeyMapper::ParseParameters {
    //! Contains true if current XML tag was successfully parsed
    bool validTag;

    DeadKeyMap *currentMap1;

    const QString *fileName;

    ParseParameters();
};

HwKbDeadKeyMapper::ParseParameters::ParseParameters()
    : validTag(true),
      currentMap1(0),
      fileName(0)
{
}

HwKbDeadKeyMapper::HwKbDeadKeyMapper()
    : xkbPrimaryLayoutSetting(XkbLayoutSettingName),
      xkbPrimaryVariantSetting(XkbVariantSettingName)
{
    (void)loadDeadKeyMaps(DeadKeyMapFile);
    connect(&xkbPrimaryLayoutSetting, SIGNAL(valueChanged()), this, SLOT(syncLayoutAndVariant()));
    connect(&xkbPrimaryVariantSetting, SIGNAL(valueChanged()), this, SLOT(syncLayoutAndVariant()));
    syncLayoutAndVariant();
}

HwKbDeadKeyMapper::~HwKbDeadKeyMapper()
{
    foreach (DeadKeyMap *deadKeyMap, deadKeyMaps) {
        qDeleteAll(*deadKeyMap);
    }
    qDeleteAll(deadKeyMaps);
}

void HwKbDeadKeyMapper::syncLayoutAndVariant()
{
    setLayout(xkbPrimaryLayoutSetting.value().toString(),
              xkbPrimaryVariantSetting.value().toString());
}

void HwKbDeadKeyMapper::setLayout(const QString &layout, const QString &variant)
{
    const QString mapKey(layout + (variant.isEmpty() ? QString("") : (QString("_") + variant)));
    currentMap = deadKeyMaps.value(mapKey);
}

bool HwKbDeadKeyMapper::filterKeyPress(QString &text, bool noCombine)
{
    const QChar previousDeadKey(deadKey);
    bool combined(false);

    if ((text.length() != 1) || !currentMap) {
        deadKey = QChar();
    } else if (currentMap->contains(text[0])) {
        deadKey = text[0];
    } else if (!deadKey.isNull()) {
        const CombineWithMap *mapping2(currentMap->value(deadKey));
        Q_ASSERT(mapping2);

        if (mapping2 && !noCombine) {
            const QChar result(text[0] == ' ' ? deadKey : mapping2->value(text[0]));
            if (!result.isNull()) {
                combined = true;
                text = result;
            }
        }
        deadKey = QChar();
    }

    if (previousDeadKey != deadKey) {
        emit stateChanged(deadKey);
    }

    return combined;
}

QChar HwKbDeadKeyMapper::currentDeadKey() const
{
    return deadKey;
}

void HwKbDeadKeyMapper::reset()
{
    QString empty;
    (void)filterKeyPress(empty);
}


// XML loading...............................................................

bool HwKbDeadKeyMapper::loadDeadKeyMaps(const QString &fileName)
{
    HwKbDeadKeyMapper::ParseParameters params;

    return loadDeadKeyMapsImpl(fileName, params);
}

bool HwKbDeadKeyMapper::loadDeadKeyMapsImpl(const QString &fileName,
                                            HwKbDeadKeyMapper::ParseParameters &params)
{
    const QString absoluteFileName = DeadKeyMapDirectory + fileName;

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
        qWarning() << "Unable to open HwKb dead key map file" << absoluteFileName;
        return false;
    }
    if (!doc.setContent((QIODevice *)(&infile), true, &errorStr, &errorLine,
                        &errorColumn)) {
        qWarning() << "Invalid HwKb dead key map file" << absoluteFileName;
        qWarning("Parse error on line %d column %d: %s", errorLine, errorColumn,
                 qPrintable(errorStr));
        return false;
    }

    bool valid = true;
    const QDomElement root = doc.documentElement();

    //check the root tag
    if (!root.isNull() && root.tagName() != HWKBTagMappings) {
        qWarning() << "Invalid hwkb dead key map file" << absoluteFileName;
        valid = false;
    } else {
        parseChildren(root, params, HWKBTagLanguage, &HwKbDeadKeyMapper::parseTagLanguage);
        valid = params.validTag;
    }

    return valid;

}

void HwKbDeadKeyMapper::parseChildren(const QDomElement &element,
                                      HwKbDeadKeyMapper::ParseParameters &params,
                                      const char *tag1, TagParser parser1,
                                      const char *tag2, TagParser parser2)
{
    Q_ASSERT(tag1);
    Q_ASSERT(parser1);

    for (QDomNode child = element.firstChild(); !child.isNull() && params.validTag;
            child = child.nextSibling()) {
        if (child.isElement()) {
            const QDomElement childElement = child.toElement();
            if (childElement.tagName() == tag1) {
                (this->*parser1)(childElement, params);
            } else if ((tag2 != NULL) && (childElement.tagName() == tag2)) {
                Q_ASSERT(parser2 != 0);
                (this->*parser2)(childElement, params);
            } else {
                qWarning() << "Unexpected tag" << childElement.tagName() << "on line"
                           << childElement.lineNumber() << "column" << childElement.columnNumber()
                           << "in hwkbd dead key map file" << *params.fileName;
                qWarning() << "The only allowed tag is" << *tag1;
                params.validTag = false;
            }
        }
    }
}

void HwKbDeadKeyMapper::parseTagLanguage(const QDomElement &element,
                                         HwKbDeadKeyMapper::ParseParameters &params)
{
    params.currentMap1 = 0;
    const QString layout(element.attribute(HWKBTagLayout));
    const QString variant(element.attribute(HWKBTagVariant));
    const QString mapKey(layout + (variant.isEmpty() ? QString("") : (QString("_") + variant)));
    if (deadKeyMaps.contains(mapKey)) {
        qWarning() << "Duplicate layout/variant entry:" << layout << "," << variant
                   << "on line" << element.lineNumber() << "column" << element.columnNumber()
                   << "in dead key mapping file" << *params.fileName;
        params.currentMap1 = deadKeyMaps[mapKey];
        foreach (CombineWithMap *value, *params.currentMap1) {
            value->clear();
        }
        qDeleteAll(*params.currentMap1);
        params.currentMap1->clear();
    } else {
        params.currentMap1 = new DeadKeyMap;
        deadKeyMaps.insert(mapKey, params.currentMap1);
    }

    parseChildren(element, params, HWKBTagMapping, &HwKbDeadKeyMapper::parseTagMapping,
                  HWKBTagInclude, &HwKbDeadKeyMapper::parseTagInclude);
}

void HwKbDeadKeyMapper::parseTagMapping(const QDomElement &element,
                                        HwKbDeadKeyMapper::ParseParameters &params)
{
    const QString result = element.attribute(HWKBTagResult);
    const QString from = element.attribute(HWKBTagFrom);
    if (result.length() != 1 || from.length() != 2) {
        qWarning() << "Invalid dead key mapping from" << from << "to" << result
                   << "on line" << element.lineNumber() << "column" << element.columnNumber()
                   << "in dead key mapping file" << *params.fileName;
        params.validTag = false;
        return;
    }
    const DeadKeyMap::iterator deadKeyMappingIter(params.currentMap1->find(from[0]));
    CombineWithMap *combineWithMapping(0);
    if (deadKeyMappingIter == params.currentMap1->end()) {
        combineWithMapping = new CombineWithMap;
        params.currentMap1->insert(from[0], combineWithMapping);
    } else {
        combineWithMapping = deadKeyMappingIter.value();
    }
    combineWithMapping->insert(from[1], result[0]);
}

void HwKbDeadKeyMapper::parseTagInclude(const QDomElement &element,
                                        HwKbDeadKeyMapper::ParseParameters &params)
{
    const QString layout(element.attribute(HWKBTagLayout));
    const QString variant(element.attribute(HWKBTagVariant));
    const QString mapKey(layout + (variant.isEmpty() ? QString("") : (QString("_") + variant)));
    if (!deadKeyMaps.contains(mapKey)) {
        qWarning() << "No mapping for layout" << layout << "and variant" << variant
                   << "to include on line" << element.lineNumber()
                   << "column" << element.columnNumber()
                   << "in dead key mapping file" << *params.fileName;
        params.validTag = false;
        return;
    }

    const DeadKeyMap &deadKeyMap(*deadKeyMaps.value(mapKey));
    // For each Dead key -> CombineWithMap in the source map...
    for (DeadKeyMap::const_iterator deadKeyMapIter(deadKeyMap.begin());
         deadKeyMapIter != deadKeyMap.end(); ++deadKeyMapIter) {
        // find or create CombineWithMap in target map...
        CombineWithMap *targetCombineWithMap(0);
        const DeadKeyMap::const_iterator targetDeadKeyMapIter(
            params.currentMap1->find(deadKeyMapIter.key()));
        if (targetDeadKeyMapIter == params.currentMap1->end()) {
            targetCombineWithMap = new CombineWithMap;
            params.currentMap1->insert(deadKeyMapIter.key(), targetCombineWithMap);
        } else {
            targetCombineWithMap = targetDeadKeyMapIter.value();
        }

        // and copy source CombineWithMap content to target
        const CombineWithMap &sourceCombineWithMap(*deadKeyMapIter.value());
        for (CombineWithMap::const_iterator sourceCombineWithMapIter(sourceCombineWithMap.begin());
             sourceCombineWithMapIter != sourceCombineWithMap.end(); ++sourceCombineWithMapIter) {
            targetCombineWithMap->insert(sourceCombineWithMapIter.key(),
                                         sourceCombineWithMapIter.value());
        }
    }
}
