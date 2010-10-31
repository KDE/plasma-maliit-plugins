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



#include "layoutsmanager.h"
#include <algorithm>
#include <QDebug>

namespace
{
    const QString InputMethodLayouts("/meegotouch/inputmethods/virtualkeyboard/layouts");
    const QString NumberFormatSettingName("/meegotouch/inputmethods/numberformat");
    const QString InputMethodDefaultLayout("/meegotouch/inputmethods/virtualkeyboard/layouts/default");
    const QString XkbLayoutSettingName("/meegotouch/inputmethods/hwkeyboard/layout");
    const QString XkbVariantSettingName("/meegotouch/inputmethods/hwkeyboard/variant");
    const QString XkbSecondaryLayoutSettingName("/meegotouch/inputmethods/hwkeyboard/secondarylayout");
    const QString XkbModelSettingName("/meegotouch/inputmethods/hwkeyboard/model");
    const QString XkbSecondaryVariantSettingName("/meegotouch/inputmethods/hwkeyboard/secondaryvariant");
    const QString HardwareKeyboardAutoCapsDisabledLayouts("/meegotouch/inputmethods/hwkeyboard/autocapsdisabledlayouts");
    const QString DefaultHardwareKeyboardAutoCapsDisabledLayout("ara"); // Uses xkb layout name. Arabic is "ara".
    const QString SystemDisplayLanguage("/meegotouch/i18n/language");
    const QString DefaultNumberFormat("latin");
    const QString FallbackLayout("en_gb.xml");
    const QString FallbackXkbLayout("us");
    const QString NumberKeyboardFileArabic("number_ar.xml");
    const QString NumberKeyboardFileLatin("number.xml");
    const QString PhoneNumberKeyboardFileArabic("phonenumber_ar.xml");
    const QString PhoneNumberKeyboardFileLatin("phonenumber.xml");
    const QString PhoneNumberKeyboardFileRussian("phonenumber_ru.xml");
    const QString SymbolKeyboardFileUs("hwsymbols_us.xml");
    const QString SymbolKeyboardFileEuro("hwsymbols_euro.xml");
    const QString SymbolKeyboardFileArabic("hwsymbols_arabic.xml");
    const QString SymbolKeyboardFileChinese("hwsymbols_chinese.xml");
    const QString FallbackXkbModel("evdev");
}

LayoutsManager *LayoutsManager::Instance = 0;


// FIXME: Style of MImAbstractKeyAreas should not be needed in classes that deal with data models.
LayoutsManager::LayoutsManager()
    : configLayouts(InputMethodLayouts),
      xkbModelSetting(XkbModelSettingName),
      numberFormatSetting(NumberFormatSettingName),
      numberFormat(NumLatin),
      currentHwkbLayoutType(InvalidHardwareKeyboard)
{
    // Read settings for the first time and load keyboard layouts.
    syncLayouts();
    initXkbMap();
    syncHardwareKeyboard();
    syncNumberKeyboards();

    // Synchronize with settings when someone changes them (e.g. via control panel).
    connect(&configLayouts, SIGNAL(valueChanged()), this, SLOT(syncLayouts()));
    connect(&configLayouts, SIGNAL(valueChanged()), this, SIGNAL(selectedLayoutsChanged()));

    connect(&numberFormatSetting, SIGNAL(valueChanged()), this, SLOT(syncNumberKeyboards()));
    connect(&locale, SIGNAL(settingsChanged()), SLOT(syncNumberKeyboards()));
    locale.connectSettings();
}

LayoutsManager::~LayoutsManager()
{
    qDeleteAll(keyboards);
    keyboards.clear();
}

void LayoutsManager::createInstance()
{
    Q_ASSERT(!Instance);
    if (!Instance) {
        Instance = new LayoutsManager;
    }
}

void LayoutsManager::destroyInstance()
{
    Q_ASSERT(Instance);
    delete Instance;
    Instance = 0;
}

int LayoutsManager::layoutCount() const
{
    return keyboards.count();
}

QStringList LayoutsManager::layoutFileList() const
{
    // This will return layout files in alphabetical ascending order.
    // This means that order in gconf is ignored.
    QStringList layoutFiles = keyboards.keys();
    layoutFiles.sort();
    return layoutFiles;
}

const KeyboardData *LayoutsManager::keyboardByName(const QString &layoutFile) const
{
    QMap<QString, KeyboardData *>::const_iterator kbIter = keyboards.find(layoutFile);

    const KeyboardData *keyboard = NULL;
    if (kbIter != keyboards.end())
        keyboard = *kbIter;

    return keyboard;
}

QString LayoutsManager::keyboardTitle(const QString &layoutFile) const
{
    const KeyboardData *const keyboard = keyboardByName(layoutFile);
    return (keyboard ? keyboard->title() : "");
}

bool LayoutsManager::autoCapsEnabled(const QString &layoutFile) const
{
    const KeyboardData *const keyboard = keyboardByName(layoutFile);
    return (keyboard ? keyboard->autoCapsEnabled() : false);
}

QString LayoutsManager::keyboardLanguage(const QString &layoutFile) const
{
    const KeyboardData *const keyboard = keyboardByName(layoutFile);
    return (keyboard ? keyboard->language() : "");
}

const LayoutData *LayoutsManager::layout(const QString &layoutFile,
        LayoutData::LayoutType type,
        M::Orientation orientation) const
{
    const LayoutData *lm = 0;

    if (type == LayoutData::Number) {
        lm = numberKeyboard.layout(type, orientation);
    } else if (type == LayoutData::PhoneNumber) {
        lm = phoneNumberKeyboard.layout(type, orientation);
    } else {
        QMap<QString, KeyboardData *>::const_iterator kbIter = keyboards.find(layoutFile);

        if (kbIter != keyboards.end() && *kbIter) {
            lm = (*kbIter)->layout(type, orientation);
        }
    }

    if (!lm) {
        static const LayoutData empty;
        lm = &empty;
    }

    return lm;
}

const LayoutData *LayoutsManager::hardwareLayout(LayoutData::LayoutType type,
                                                 M::Orientation orientation) const
{
    return hwKeyboard.layout(type, orientation);
}

QString LayoutsManager::defaultLayoutFile() const
{
    return MGConfItem(InputMethodDefaultLayout).value(FallbackLayout).toString();
}

QString LayoutsManager::systemDisplayLanguage() const
{
    return MGConfItem(SystemDisplayLanguage).value().toString();
}

void LayoutsManager::initXkbMap()
{
    //init current xkb layout and variant.
    setXkbMap(xkbPrimaryLayout(), xkbPrimaryVariant());
}

QString LayoutsManager::xkbModel() const
{
    return xkbModelSetting.value(FallbackXkbModel).toString();
}

QString LayoutsManager::xkbLayout() const
{
    return xkbCurrentLayout;
}

QString LayoutsManager::xkbVariant() const
{
    return xkbCurrentVariant;
}

QString LayoutsManager::xkbPrimaryLayout() const
{
    return MGConfItem(XkbLayoutSettingName).value(FallbackXkbLayout).toString();
}

QString LayoutsManager::xkbPrimaryVariant() const
{
    return MGConfItem(XkbVariantSettingName).value().toString();
}

QString LayoutsManager::xkbSecondaryLayout() const
{
    return MGConfItem(XkbSecondaryLayoutSettingName).value().toString();
}

QString LayoutsManager::xkbSecondaryVariant() const
{
    return MGConfItem(XkbSecondaryVariantSettingName).value().toString();
}

void LayoutsManager::setXkbMap(const QString &layout, const QString &variant)
{
    bool changed = false;
    if (layout != xkbCurrentLayout) {
        changed = true;
        xkbCurrentLayout = layout;
    }

    if (variant != xkbCurrentVariant) {
        changed = true;
        xkbCurrentVariant = variant;
    }

    if (changed) {
        syncHardwareKeyboard();
    }
}

bool LayoutsManager::hardwareKeyboardAutoCapsEnabled() const
{
    // Arabic hwkb layout default disable autocaps.
    QStringList autoCapsDisabledLayouts = MGConfItem(HardwareKeyboardAutoCapsDisabledLayouts)
                                            .value(QStringList(DefaultHardwareKeyboardAutoCapsDisabledLayout))
                                            .toStringList();
    return !(autoCapsDisabledLayouts.contains(xkbLayout()));
}

bool LayoutsManager::loadLayout(const QString &layout)
{
    QMap<QString, KeyboardData *>::iterator kbIterator;

    // sanity tests
    if (layout.isEmpty())
        return false;

    KeyboardData *keyboard = new KeyboardData;

    bool loaded = keyboard->loadNokiaKeyboard(layout);

    if (!loaded) {
        loaded = keyboard->loadNokiaKeyboard(layout);
    }

    if (!loaded) {
        delete keyboard;
        keyboard = NULL;
        return false;
    }

    // Make sure entry for layout exists. Create if it doesn't.
    kbIterator = keyboards.find(layout);
    if (kbIterator == keyboards.end()) {
        kbIterator = keyboards.insert(layout, 0);
    }

    if (*kbIterator) {
        // What to do?
        qWarning() << "LayoutsManager: Layouts have already been loaded for layout "
                   << keyboard->layoutFile();
        delete keyboard;
        keyboard = NULL;
        return false;
    }

    *kbIterator = keyboard;

    return true;
}

void LayoutsManager::syncNumberKeyboards()
{
    const QString formatString(numberFormatSetting.value(DefaultNumberFormat).toString().toLower());
    numberFormat = NumLatin;
    if ("arabic" == formatString) {
        numberFormat = NumArabic;
    } else if ("latin" != formatString) {
        qWarning("Invalid value (%s) for number format setting (%s), using Latin.",
                 formatString.toAscii().constData(), NumberFormatSettingName.toAscii().constData());
    }

    bool loaded = numberKeyboard.loadNokiaKeyboard(
                      numberFormat == NumLatin ? NumberKeyboardFileLatin : NumberKeyboardFileArabic);
    // Fall back to Latin if Arabic not available
    if (!loaded && (numberFormat == NumArabic)) {
        numberFormat = NumLatin;
        numberKeyboard.loadNokiaKeyboard(NumberKeyboardFileLatin);
    }

    // Phone number keyboard

    loaded = false;

    if (numberFormat == NumArabic) {
        loaded = phoneNumberKeyboard.loadNokiaKeyboard(PhoneNumberKeyboardFileArabic);
    } else if (locale.categoryLanguage(MLocale::MLcMessages) == "ru") {
        loaded = phoneNumberKeyboard.loadNokiaKeyboard(PhoneNumberKeyboardFileRussian);
    }

    if (!loaded) {
        phoneNumberKeyboard.loadNokiaKeyboard(PhoneNumberKeyboardFileLatin);
    }
    emit numberFormatChanged();
}

void LayoutsManager::syncLayouts()
{
    bool changed = false;
    QStringList newLayouts;
    const QStringList oldLayouts = layoutFileList();

    // Add new ones
    if (!configLayouts.value().isNull()) {
        newLayouts = configLayouts.value().toStringList();

        foreach(QString layoutFile, newLayouts) {
            // Existing layouts are not reloaded.
            if (!oldLayouts.contains(layoutFile, Qt::CaseInsensitive)) {
                // Add new layout
                if (!loadLayout(layoutFile)) {
                    qWarning() << __PRETTY_FUNCTION__
                               << "New layout file " << layoutFile << " could not be loaded.";
                } else {
                    changed = true;
                }
            }
        }
    }

    // Remove old ones
    foreach(QString old, oldLayouts) {
        if (!newLayouts.contains(old, Qt::CaseInsensitive)) {
            keyboards.remove(old);
            changed = true;
        }
    }

    // Try FallbackLayout if no layouts loaded.
    // Don't try to load again if we already tried.
    if (keyboards.isEmpty() && !newLayouts.contains(FallbackLayout)) {
        if (loadLayout(FallbackLayout)) {
            changed = true;
        }
    }

    if (changed) {
        emit layoutsChanged();
    }
}

void LayoutsManager::syncHardwareKeyboard()
{
    const QString xkbName = xkbLayout();
    const HardwareKeyboardLayout hwkbLayoutType = xkbLayoutType(xkbName);

    if (hwkbLayoutType == currentHwkbLayoutType) {
        return;
    }

    currentHwkbLayoutType = hwkbLayoutType;

    // What we could do here is to load a generic hw layout xml file
    // that would import the correct symbol layout variant but since
    // symbol sections are the only things we currently use, let's just
    // load the hw symbols xml directly.
    const HardwareSymbolVariant symVariant = HwkbLayoutToSymVariant[hwkbLayoutType];
    const QString filename = symbolVariantFileName(symVariant);

    if (hwKeyboard.loadNokiaKeyboard(filename)) {
        emit hardwareLayoutChanged();
    } else {
        qWarning() << "LayoutsManager: loading of hardware layout specific keyboard "
                   << filename << " failed";
    }
}

QString LayoutsManager::symbolVariantFileName(HardwareSymbolVariant symVariant)
{
    QString symFileName;

    switch (symVariant) {
    case HwSymbolVariantUs:
        symFileName = SymbolKeyboardFileUs;
        break;
    case HwSymbolVariantArabic:
        symFileName = SymbolKeyboardFileArabic;
        break;
    case HwSymbolVariantChinese:
        symFileName = SymbolKeyboardFileChinese;
        break;
    case HwSymbolVariantEuro:
    default:
        symFileName = SymbolKeyboardFileEuro;
        break;
    }

    return symFileName;
}

QMap<QString, QString> LayoutsManager::selectedLayouts() const
{
    QMap<QString, QString> layouts;
    foreach (const QString layout, layoutFileList()) {
        layouts.insert(layout, keyboardTitle(layout));
    }
    return layouts;
}
