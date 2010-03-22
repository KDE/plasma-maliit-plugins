/* * This file is part of dui-keyboard *
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


#ifndef LAYOUTSMANAGER_H
#define LAYOUTSMANAGER_H

#include "keyboarddata.h"
#include "layoutdata.h"
#include <DuiGConfItem>
#include <DuiLocale>
#include <QMap>
#include <QObject>
#include <memory>

class LayoutsManager : public QObject
{
    Q_OBJECT

public:
    virtual ~LayoutsManager();

    //! \brief Get singleton instance
    //! \return singleton instance
    static LayoutsManager &instance();

    //! \brief Create singleton
    static void createInstance();

    //! \brief Destroy singleton
    static void destroyInstance();

    /*!
     * \brief Returns true if \a language belongs to cyrillic.
     * Cyrillic language includes Russian, Polish, Bulgaria, Serbian, Kirghiz and Ukrainian.
     * \param language Language name such as ru_RU or ru.
     */
    static bool isCyrillicLanguage(const QString &language);

    int languageCount() const;

    QStringList languageList() const;

    //! Returns title of a given language as stated in xml.
    QString keyboardTitle(const QString &language) const;

    //! \return real language (as specified in XML) of the keyboard
    //! loaded based on language list entry 'language'.
    QString keyboardLanguage(const QString &language) const;

    //! \brief Get layout model by language, type, and orientation
    const LayoutData *layout(const QString &language, LayoutData::LayoutType type,
                             Dui::Orientation orientation) const;

    //! \brief Returns currently set default language
    QString defaultLanguage() const;

    //! \brief Returns currently system display language
    QString systemDisplayLanguage() const;

    //! \brief Returns the language for hardware keyboard layout
    QString hardwareKeyboardLanguage() const;

signals:
    //! Signals that languages have been reset and keyboard data can
    //! be reloaded using new languages returned by languageList().
    void languagesChanged();

private:
    //! Default constructor
    LayoutsManager();

    /*!
     * Returns the layout of the given language
     * \param language language name such as fi_FI or fi
     * \return true if language was loaded, false if failure or language already existed
     */
    bool loadLanguage(const QString &language);

    //! \return keyboard that matches language list entry 'language'
    const KeyboardData *keyboardByName(const QString &language) const;

private slots:
    void syncLanguages();
    void reloadNumberKeyboards();

private:
    //! Valid states of number format setting
    enum NumberFormat {
        NumArabic,
        NumLatin
    };
    //! All keyboards arranged by language.
    //! The map key is language name as read from settings.
    QMap<QString, KeyboardData *> keyboards;

    //! DuiGConfItem for selected languages available for
    //! vkb's use. The settings are set by control panel applet.
    DuiGConfItem configLanguages;

    //! Setting that determines whether number format is Arabic or Latin
    DuiGConfItem numberFormatSetting;

    //! Iterator that points to current language and keyboard.
    //! Current keyboard is used when no language name is specified
    //! in calls to public methods of this class.
    QMap<QString, KeyboardData *>::const_iterator current;

    //! Current number keyboard
    KeyboardData numberKeyboard;

    //! Current phone number keyboard
    KeyboardData phoneNumberKeyboard;

    //! Current state of number format setting (represented by \a numberFormatSetting)
    NumberFormat numberFormat;

    //! System locale
    DuiLocale locale;

    //! Singleton instance
    static LayoutsManager *Instance;

    friend class Ut_LayoutsManager;
};


inline LayoutsManager &LayoutsManager::instance()
{
    Q_ASSERT(Instance);
    return *Instance;
}


#endif
