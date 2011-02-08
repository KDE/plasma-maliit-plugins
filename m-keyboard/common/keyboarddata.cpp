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



#include "keyboarddata.h"

#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFile>


namespace
{
    // TODO: Support Windows paths too.
    const char * const VKBConfigurationPath = "/usr/share/meegotouch/virtual-keyboard/layouts/";

    const char * const VKBTagKeyboard             = "keyboard";
    const char * const VKBTagVersion              = "version";
    const char * const VKBTagCatalog              = "catalog";
    const char * const VKBTagAutoCapitalization   = "autocapitalization";
    const char * const VKBTagLayout               = "layout";
    const char * const VKBTagTitle                = "title";
    const char * const VKBTagLanguage             = "language";
    const char * const VKBTagBoolTrue             = "true";
    const char * const VKBTagBoolFalse            = "false";
    const char * const VKBTagType                 = "type";
    const char * const VKBTagTypeGeneral          = "general";
    const char * const VKBTagTypeUrl              = "url";
    const char * const VKBTagTypeEmail            = "email";
    const char * const VKBTagTypeNumber           = "number";
    const char * const VKBTagTypePhoneNumber      = "phonenumber";
    const char * const VKBTagTypeCommon           = "common";
    const char * const VKBTagOrientation          = "orientation";
    const char * const VKBTagOrientationLandscape = "landscape";
    const char * const VKBTagOrientationPortrait  = "portrait";

    const char * const VKBTagID                   = "id";
    const char * const VKBTagWidth                = "width";
    const char * const VKBTagHeight               = "height";

    const char * const VKBTagTypeNonsloppy        = "non-sloppy";
    const char * const VKBTagHorizontalAlignment  = "horizontal_alignment";
    const char * const VKBTagVerticalAlignment    = "vertical_alignment";
    const char * const VKBTagAlignFull            = "full";
    const char * const VKBTagAlignLeft            = "left";
    const char * const VKBTagAlignRight           = "right";
    const char * const VKBTagAlignBottom          = "bottom";
    const char * const VKBTagAlignTop             = "top";
    const char * const VKBTagAlignCenter          = "center";

    const char * const VKBTagRow                  = "row";
    const char * const VKBTagSection              = "section";
    const char * const VKBTagMovable              = "movable";

    const char * const VKBTagBinding              = "binding";
    const char * const VKBTagKey                  = "key";
    const char * const VKBTagSpacer               = "spacer";

    const char * const VKBTagKeyAction            = "action";
    const char * const VKBTagShift                = "shift";
    const char * const VKBTagLabel                = "label";
    const char * const VKBTagSecondaryLabel       = "secondary_label";
    const char * const VKBTagAccents              = "accents";
    const char * const VKBTagAccentedLabels       = "accented_labels";
    const char * const VKBTagExtendedLabels       = "extended_labels";
    const char * const VKBTagCycleSet             = "cycleset";
    const char * const VKBTagDead                 = "dead";
    const char * const VKBTagQuickPick            = "quickpick";

    const char * const ActionStrInsert            = "insert";
    const char * const ActionStrShift             = "shift";
    const char * const ActionStrBackspace         = "backspace";
    const char * const ActionStrSpace             = "space";
    const char * const ActionStrCycle             = "cycle";
    const char * const ActionStrLayoutMenu        = "layout_menu";
    const char * const ActionStrSym               = "sym";
    const char * const ActionStrReturn            = "return";
    const char * const ActionStrDecimalSeparator  = "decimal_separator";
    const char * const ActionStrPlusMinusToggle   = "plus_minus_toggle";
    const char * const ActionStrTab               = "tab";
    const char * const ActionStrCommit            = "commit";
    const char * const ActionStrSwitch            = "switch";

    const char * const VKBTagImport               = "import";
    const char * const VKBTagFile                 = "file";

    const char * const RtlString                  = "rtl";
    const char * const RtlStringDefValue          = "false";

    const char * const StyleString                = "style";
    const char * const StyleStringDefValue        = "normal";
    const char * const WidthTypeString            = "width";
    const char * const WidthTypeStringDefValue    = "medium";
    const char * const FixedString                = "fixed";
    const char * const FixedStringDefValue        = "false";
    const char * const HeightTypeString           = "height";
    const char * const HeightTypeStringDefValue   = "medium";
    const char * const KeyIdString                = "id";
}

struct ParseParameters {
    MImKeyModel *currentKey;

    //! New rows will be added to currentSection
    LayoutData::SharedLayoutSection currentSection;

    //! New keys will be added to currentRow
    LayoutSection::Row *currentRow;

    //! Contains true if current XML tag was successfully parsed
    bool validTag;

    const QString *fileName;

    ParseParameters();
};

ParseParameters::ParseParameters():
    currentKey(0),
    currentSection(0),
    currentRow(0),
    validTag(true),
    fileName(0)
{
}

KeyboardData::KeyboardData()
    : currentLayout(0),
      keyboardVersion(""),
      keyboardTitle(""),
      keyboardLanguage(""),
      keyboardCatalog(""),
      keyboardAutoCapsEnabled(true)
{
    layoutTypeMap[VKBTagTypeGeneral] = LayoutData::General;
    layoutTypeMap[VKBTagTypeUrl] = LayoutData::Url;
    layoutTypeMap[VKBTagTypeEmail] = LayoutData::Email;
    layoutTypeMap[VKBTagTypeNumber] = LayoutData::Number;
    layoutTypeMap[VKBTagTypePhoneNumber] = LayoutData::PhoneNumber;
    layoutTypeMap[VKBTagTypeCommon] = LayoutData::Common;
}

KeyboardData::~KeyboardData()
{
    qDeleteAll(layouts);
    layouts.clear();
}

inline MImKeyModel::StyleType KeyboardData::toStyleType(const QString &attributeValue)
{
    MImKeyModel::StyleType type = MImKeyModel::NormalStyle;

    if (attributeValue == "special") {
        type = MImKeyModel::SpecialStyle;
    } else if (attributeValue == "deadkey") {
        type = MImKeyModel::DeadkeyStyle;
    }

    return type;
}

inline MImKeyModel::WidthType KeyboardData::toWidthType(const QString &attributeValue)
{
    MImKeyModel::WidthType widthType = MImKeyModel::Medium;

    if (attributeValue == "small") {
        widthType = MImKeyModel::Small;
    } else  if (attributeValue == "large") {
        widthType = MImKeyModel::Large;
    } else  if (attributeValue == "x-large") {
        widthType = MImKeyModel::XLarge;
    } else  if (attributeValue == "xx-large") {
        widthType = MImKeyModel::XxLarge;
    } else  if (attributeValue == "stretched") {
        widthType = MImKeyModel::Stretched;
    }

    return widthType;
}

inline LayoutSection::RowHeightType KeyboardData::toHeightType(const QString &attributeValue)
{
    LayoutSection::RowHeightType heightType = LayoutSection::Medium;

    if (attributeValue == "small") {
        heightType = LayoutSection::Small;
    } else  if (attributeValue == "large") {
        heightType = LayoutSection::Large;
    } else  if (attributeValue == "x-large") {
        heightType = LayoutSection::XLarge;
    } else  if (attributeValue == "xx-large") {
        heightType = LayoutSection::XxLarge;
    }
    return heightType;
}

inline bool KeyboardData::toBoolean(const QString &attributeValue)
{
    return attributeValue == "true" || attributeValue == "1";
}

const LayoutData *KeyboardData::layout(LayoutData::LayoutType type,
                                       M::Orientation orientation,
                                       bool portaitFallback) const
{
    return layoutPrivate(type, orientation, portaitFallback);
}

LayoutData *KeyboardData::layoutPrivate(LayoutData::LayoutType type,
                                        M::Orientation orientation,
                                        bool portraitFallback) const
{
    LayoutData *bestMatch = NULL;
    foreach(LayoutData * layoutModel, layouts) {
        if (layoutModel->type() == type) {
            if (layoutModel->orientation() == orientation) {
                return layoutModel;
            } else if (portraitFallback) {
                // Use a nearest match
                bestMatch = layoutModel;
            }
        }
    }
    return bestMatch;
}

QString KeyboardData::language() const
{
    return keyboardLanguage;
}

QString KeyboardData::title() const
{
    return keyboardTitle;
}

QString KeyboardData::layoutFile() const
{
    return layoutFileName;
}

bool KeyboardData::autoCapsEnabled() const
{
    return keyboardAutoCapsEnabled;
}

bool KeyboardData::loadNokiaKeyboard(const QString &fileName)
{
    ParseParameters params;

    // allow reloading
    qDeleteAll(layouts);
    layouts.clear();

    layoutFileName = fileName;

    const bool success = loadNokiaKeyboardImpl(fileName, params);
    if (!success) {
        qDeleteAll(layouts);
        layouts.clear();
    }
    return success;
}

bool KeyboardData::loadNokiaKeyboardImpl(const QString &fileName, ParseParameters &params,
        bool importedLayout)
{
    QString absoluteFileName;
    if (QDir::isAbsolutePath(fileName)) {
        absoluteFileName = fileName;
    } else {
        absoluteFileName = VKBConfigurationPath + fileName;
    }

    params.fileName = &absoluteFileName;
    if (!QFile::exists(absoluteFileName)) {
        qWarning() << "Virtual keyboard layout file" << absoluteFileName << "does not exist.";
        return false;
    }

    QFile infile(absoluteFileName);
    QString errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument doc;

    if (!infile.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open virtual keyboard layout file" << absoluteFileName;
        return false;
    }
    if (!doc.setContent((QIODevice *)(&infile), true, &errorStr, &errorLine,
                        &errorColumn)) {
        qWarning() << "Invalid virtual keyboard layout file" << absoluteFileName;
        qWarning("Parse error on line %d column %d: %s", errorLine, errorColumn,
                 errorStr.toAscii().constData());
        return false;
    }

    bool valid = true;
    const QDomElement root = doc.documentElement();

    //check the root tag
    if (!root.isNull() && root.tagName() != VKBTagKeyboard) {
        qWarning() << "Invalid virtual keyboard layout file" << absoluteFileName;
        valid = false;
    } else {
        // Don't overwrite keyboard layout information when using <import>
        if (!importedLayout) {
            keyboardVersion = root.attribute(VKBTagVersion);
            keyboardTitle = root.attribute(VKBTagTitle);
            keyboardLanguage = root.attribute(VKBTagLanguage);
            keyboardCatalog = root.attribute(VKBTagCatalog);
            keyboardAutoCapsEnabled = toBoolean(root.attribute(VKBTagAutoCapitalization, "true"));
        }

        parseChildren(root, params, VKBTagImport, &KeyboardData::parseTagImport,
                      VKBTagLayout, &KeyboardData::parseTagLayout);
        valid = params.validTag;
    }

    return valid;
}

/*
 * parseChildren has been hardwired to support max. two different tags at each level,
 * which is sufficient for now and makes this easier to use.  If more will be needed,
 * change this to accept an array of (tagname, parser) structures.
 */
void KeyboardData::parseChildren(const QDomElement &element, ParseParameters &params,
                                 const char * const tag1, TagParser parser1, const char * const tag2,
                                 TagParser parser2)
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
                           << "in layout file" << *params.fileName;
                if (tag2) {
                    qWarning() << "Allowed tags are" << tag1 << "and" << tag2;
                } else {
                    qWarning() << "The only allowed tag is" << tag1;
                }
                params.validTag = false;
            }
        }
    }
}

void KeyboardData::parseTagImport(const QDomElement &element, ParseParameters &params)
{
    const QString redirectFileName = element.attribute(VKBTagFile);
    const QString absoluteFileName = VKBConfigurationPath + redirectFileName;
    qDebug() << "Importing file" << absoluteFileName;
    if (QFile::exists(absoluteFileName)) {
        const QString *savedFileName = params.fileName;
        if (!loadNokiaKeyboardImpl(redirectFileName, params, true)) {
            qWarning() << __PRETTY_FUNCTION__
                       << "wrong format xml" << absoluteFileName << "for virtual keyboard";
            params.validTag = false;
        }
        params.fileName = savedFileName;
    }
}

M::Orientation KeyboardData::orientation(const QString &orientationString)
{
    M::Orientation orient = M::Portrait;
    if (orientationString == VKBTagOrientationLandscape)
        orient = M::Landscape;
    return orient;
}

void KeyboardData::parseTagLayout(const QDomElement &element, ParseParameters &params)
{
    const QString typeString = element.attribute(VKBTagType);
    LayoutData::LayoutType type = LayoutData::General;

    // Convert type string to internal representation
    if (layoutTypeMap.contains(typeString)) {
        type = layoutTypeMap.value(typeString);
    } else {
        params.validTag = false;
        qWarning() << __PRETTY_FUNCTION__ << "invalid layout type";
        return;
    }

    // Inherit sections from a layout of same type and same orientation
    // (->just use that layout model), if available, or from a layout of
    // same type and different orientation (copy sections to a new
    // layout model), if available.

    const M::Orientation orient = orientation(element.attribute(VKBTagOrientation));
    LayoutData *layoutModel = layoutPrivate(type, orient, false);

    if (layoutModel == NULL) {
        layoutModel = new LayoutData;

        layoutModel->layoutType = type;
        layoutModel->layoutOrientation = orient;

        M::Orientation otherOrientation;
        if (orient == M::Portrait) {
            otherOrientation = M::Landscape;
        } else {
            otherOrientation = M::Portrait;
        }
        const LayoutData *const parentLayout = layoutPrivate(type, otherOrientation, false);
        if (parentLayout) {
            layoutModel->sectionMap = parentLayout->sectionMap;
        }

        layouts.append(layoutModel);
    }

    currentLayout = layoutModel;

    parseChildren(element, params, VKBTagSection, &KeyboardData::parseTagSection);

    currentLayout->sections = currentLayout->sectionMap.values();
}

Qt::Alignment KeyboardData::alignment(const QString &alignmentString, bool vertical)
{
    Qt::Alignment align = Qt::AlignJustify;
    if (alignmentString == VKBTagAlignFull)
        align = Qt::AlignJustify;
    else if (alignmentString == VKBTagAlignTop)
        align = Qt::AlignTop;
    else if (alignmentString == VKBTagAlignBottom)
        align = Qt::AlignBottom;
    else if (alignmentString == VKBTagAlignCenter)
        align = vertical ? Qt::AlignVCenter : Qt::AlignHCenter;
    else if (alignmentString == VKBTagAlignLeft)
        align = Qt::AlignLeft;
    else if (alignmentString == VKBTagAlignRight)
        align = Qt::AlignRight;
    return align;
}

void KeyboardData::parseTagSection(const QDomElement &element, ParseParameters &params)
{
    LayoutData::SharedLayoutSection section(new LayoutSection);
    section->movable = toBoolean(element.attribute(VKBTagMovable));
    section->sectionName = element.attribute(VKBTagID);
    section->sectionType = (element.attribute(VKBTagType) == VKBTagTypeNonsloppy) ? LayoutSection::NonSloppy : LayoutSection::Sloppy;
    params.currentSection = section;
    currentLayout->sectionMap.insert(section->sectionName, section);
    parseChildren(element, params, VKBTagRow, &KeyboardData::parseTagRow);
}

void KeyboardData::parseTagRow(const QDomElement &element, ParseParameters &params)
{
    LayoutSection::Row *row = new LayoutSection::Row;
    row->heightType = toHeightType(element.attribute(HeightTypeString, HeightTypeStringDefValue)); 
    params.currentSection->rows.append(row);
    params.currentRow = row;

    parseChildren(element, params,
                  VKBTagKey, &KeyboardData::parseTagKey,
                  VKBTagSpacer, &KeyboardData::parseTagSpacer);

    params.currentSection->mMaxColumns = qMax(params.currentSection->maxColumns(),
                                              row->keys.size());
}

void KeyboardData::parseTagBinding(const QDomElement &element, ParseParameters &params)
{
    const bool shift = toBoolean(element.attribute(VKBTagShift, "false"));

    if (params.currentKey->bindings[shift ? MImKeyModel::Shift : MImKeyModel::NoShift]) {
        qWarning() << "Ignoring duplicate binding with same shift attribute on line"
                   << element.lineNumber() << "column" << element.columnNumber()
                   << "in layout file" << *params.fileName;
        return;
    }

    MImKeyBinding *binding = new MImKeyBinding;

    binding->keyLabel = element.attribute(VKBTagLabel);
    binding->keyAction = keyActionFromString(element.attribute(VKBTagKeyAction));
    if (binding->keyAction == MImKeyBinding::ActionCycle) {
        binding->cycleSet = element.attribute(VKBTagCycleSet);
    }
    binding->secondary_label = element.attribute(VKBTagSecondaryLabel);
    binding->dead = toBoolean(element.attribute(VKBTagDead));
    binding->quickPick = toBoolean(element.attribute(VKBTagQuickPick));

    binding->accents = element.attribute(VKBTagAccents);
    binding->accented_labels = element.attribute(VKBTagAccentedLabels);

    binding->extended_labels = element.attribute(VKBTagExtendedLabels);

    params.currentKey->bindings[shift ? MImKeyModel::Shift : MImKeyModel::NoShift] = binding;
}

void KeyboardData::parseTagKey(const QDomElement &element, ParseParameters &params)
{
    MImKeyModel::StyleType type = toStyleType(element.attribute(StyleString, StyleStringDefValue));
    MImKeyModel::WidthType widthType = toWidthType(element.attribute(WidthTypeString, WidthTypeStringDefValue));
    const bool isRtl = toBoolean(element.attribute(RtlString, RtlStringDefValue));
    const bool isFixed = toBoolean(element.attribute(FixedString, FixedStringDefValue));
    const QString keyId = element.attribute(KeyIdString);

    MImKeyModel *key = new MImKeyModel(type, widthType, isFixed, isRtl, keyId);
    params.currentKey = key;
    params.currentRow->keys.append(key);

    parseChildren(element, params, VKBTagBinding, &KeyboardData::parseTagBinding);

    if (key->bindings[1] == NULL) {
        key->bindings[1] = key->bindings[0];
    }

    if (key->bindings[0] == NULL) {
        key->bindings[0] = key->bindings[1];
    }
}

void KeyboardData::parseTagSpacer(const QDomElement &, ParseParameters &params)
{
    params.currentRow->spacerIndices.append(params.currentRow->keys.count() - 1);
}

MImKeyBinding::KeyAction KeyboardData::keyActionFromString(const QString &typeStr)
{
    MImKeyBinding::KeyAction result;

    if (typeStr == ActionStrShift)
        result = MImKeyBinding::ActionShift;
    else if (typeStr == ActionStrInsert)
        result = MImKeyBinding::ActionInsert;
    else if (typeStr == ActionStrBackspace)
        result = MImKeyBinding::ActionBackspace;
    else if (typeStr == ActionStrSpace)
        result = MImKeyBinding::ActionSpace;
    else if (typeStr == ActionStrCycle)
        result = MImKeyBinding::ActionCycle;
    else if (typeStr == ActionStrLayoutMenu)
        result = MImKeyBinding::ActionLayoutMenu;
    else if (typeStr == ActionStrSym)
        result = MImKeyBinding::ActionSym;
    else if (typeStr == ActionStrReturn)
        result = MImKeyBinding::ActionReturn;
    else if (typeStr == ActionStrDecimalSeparator)
        result = MImKeyBinding::ActionDecimalSeparator;
    else if (typeStr == ActionStrPlusMinusToggle)
        result = MImKeyBinding::ActionPlusMinusToggle;
    else if (typeStr == ActionStrTab)
        result = MImKeyBinding::ActionTab;
    else if (typeStr == ActionStrCommit)
        result = MImKeyBinding::ActionCommit;
    else if (typeStr == ActionStrSwitch)
        result = MImKeyBinding::ActionSwitch;
    else
        result = MImKeyBinding::ActionInsert;
    return result;
}
