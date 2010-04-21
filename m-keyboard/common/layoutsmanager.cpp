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



#include "layoutsmanager.h"
#include <algorithm>
#include <QDebug>

namespace
{
    const QString InputMethodLanguages("/meegotouch/inputmethods/languages");
    const QString NumberFormatSettingName("/meegotouch/inputmethods/numberformat");
    const QString InputMethodDefaultLanguage("/meegotouch/inputmethods/languages/default");
    const QString HardwareKeyboardLayout("/meegotouch/inputmethods/hwkeyboard/layout");
    const QString SystemDisplayLanguage("/meegotouch/i18n/language");
    const QString DefaultNumberFormat("latin");
    const QString LayoutFileExtension(".xml");
    const QString FallbackLanguage("en");
    const QString NumberKeyboardFileArabic("number_ar.xml");
    const QString NumberKeyboardFileLatin("number.xml");
    const QString PhoneNumberKeyboardFileArabic("phonenumber_ar.xml");
    const QString PhoneNumberKeyboardFileLatin("phonenumber.xml");
    const QString PhoneNumberKeyboardFileRussian("phonenumber_ru.xml");
}

LayoutsManager *LayoutsManager::Instance = 0;


LayoutsManager::LayoutsManager()
    : configLanguages(InputMethodLanguages),
      numberFormatSetting(NumberFormatSettingName),
      numberFormat(NumLatin)
{
    // Read settings for the first time and load keyboard layouts.
    syncLanguages();
    reloadNumberKeyboards();

    // Synchronize with settings when someone changes them (e.g. via control panel).
    connect(&configLanguages, SIGNAL(valueChanged()), this, SLOT(syncLanguages()));
    connect(&numberFormatSetting, SIGNAL(valueChanged()), this, SLOT(reloadNumberKeyboards()));
    connect(&locale, SIGNAL(settingsChanged()), SLOT(reloadNumberKeyboards()));
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

int LayoutsManager::languageCount() const
{
    return keyboards.count();
}

QStringList LayoutsManager::languageList() const
{
    // This will return languages in alphabetical ascending order.
    // This means that order in gconf is ignored.
    return keyboards.keys();
}

const KeyboardData *LayoutsManager::keyboardByName(const QString &language) const
{
    QMap<QString, KeyboardData *>::const_iterator kbIter = keyboards.find(language);

    const KeyboardData *keyboard = NULL;
    if (kbIter != keyboards.end())
        keyboard = *kbIter;

    return keyboard;
}

QString LayoutsManager::keyboardTitle(const QString &language) const
{
    const KeyboardData *keyboard = keyboardByName(language);
    return keyboard == NULL ? "" : keyboard->title();
}

QString LayoutsManager::keyboardLanguage(const QString &language) const
{
    const KeyboardData *keyboard = keyboardByName(language);
    return keyboard == NULL ? "" : keyboard->language();
}

const LayoutData *LayoutsManager::layout(const QString &language,
        LayoutData::LayoutType type,
        M::Orientation orientation) const
{
    const LayoutData *lm = NULL;

    if (type == LayoutData::Number) {
        lm = numberKeyboard.layout(type, orientation);
    } else if (type == LayoutData::PhoneNumber) {
        lm = phoneNumberKeyboard.layout(type, orientation);
    } else {
        QMap<QString, KeyboardData *>::const_iterator kbIter = keyboards.find(language);

        if (kbIter != keyboards.end() && *kbIter) {
            // Look for correct layout
            lm = (*kbIter)->layout(type, orientation);
        }
        // TODO: if (lm == NULL) search other languages as a last resort?
    }

    return lm;
}

QString LayoutsManager::defaultLanguage() const
{
    return MGConfItem(InputMethodDefaultLanguage).value(FallbackLanguage).toString();
}

QString LayoutsManager::systemDisplayLanguage() const
{
    return MGConfItem(SystemDisplayLanguage).value().toString();
}

QString LayoutsManager::hardwareKeyboardLanguage() const
{
    //TODO: this is a temporary method, should get the hw layout, then return the
    //language (symbol view variant) according Table 4 in HW Keyboard UI spec.
    return MGConfItem(HardwareKeyboardLayout).value(FallbackLanguage).toString();
}

bool LayoutsManager::loadLanguage(const QString &language)
{
    QMap<QString, KeyboardData *>::iterator kbIterator;

    // sanity tests
    if (language.isEmpty())
        return false;

    KeyboardData *keyboard = new KeyboardData;

    bool loaded = keyboard->loadNokiaKeyboard(language + LayoutFileExtension);

    if (!loaded) {
        // In case of "en_GB" we'll try to load en.xml.
        if (KeyboardData::isLanguageLongFormat(language)) {
            loaded = keyboard->loadNokiaKeyboard(
                         KeyboardData::convertLanguageToShortFormat(language) + LayoutFileExtension);
        }
    }

    if (!loaded) {
        delete keyboard;
        keyboard = NULL;
        return false;
    }

    // Make sure entry for language exists. Create if it doesn't.
    kbIterator = keyboards.find(language);
    if (kbIterator == keyboards.end()) {
        kbIterator = keyboards.insert(language, 0);
    }

    if (*kbIterator) {
        // What to do?
        qWarning() << "LayoutsManager: Layouts have already been loaded for language "
                   << keyboard->language();
        delete keyboard;
        keyboard = NULL;
        return false;
    }

    *kbIterator = keyboard;

    return true;
}

void LayoutsManager::reloadNumberKeyboards()
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

void LayoutsManager::syncLanguages()
{
    bool changed = false;
    QStringList newLanguages;
    const QStringList oldLanguages = languageList();

    // Add new ones
    if (!configLanguages.value().isNull()) {
        newLanguages = configLanguages.value().toStringList();

        foreach(QString language, newLanguages) {
            // Existing languages are not reloaded.
            if (!keyboards.contains(language)) {
                // Add new language
                if (!loadLanguage(language)) {
                    qWarning() << __PRETTY_FUNCTION__
                               << "New language " << language << " could not be loaded.";
                } else {
                    changed = true;
                }
            }
        }
    }

    // Remove old ones
    foreach(QString old, oldLanguages) {
        if (!newLanguages.contains(old)) {
            keyboards.remove(old);
            changed = true;
        }
    }

    // Try FallbackLanguage if no languages loaded.
    // Don't try to load again if we already tried.
    if (keyboards.isEmpty() && !newLanguages.contains(FallbackLanguage)) {
        if (loadLanguage(FallbackLanguage)) {
            changed = true;
        }
    }

    if (changed) {
        emit languagesChanged();
    }
}

bool LayoutsManager::isCyrillicLanguage(const QString &language)
{
    bool val = false;

    QString shortFormatLanguage = language;
    if (KeyboardData::isLanguageLongFormat(language))
        shortFormatLanguage = KeyboardData::convertLanguageToShortFormat(language);
    if (shortFormatLanguage == "ru"    // Russian
            || shortFormatLanguage == "pl" // Polish
            || shortFormatLanguage == "bg" // Bulgaria
            || shortFormatLanguage == "sr" // Serbian
            || shortFormatLanguage == "ky" // Kirghiz
            || shortFormatLanguage == "uk" // Ukrainian
       )
        val = true;
    return val;
}
