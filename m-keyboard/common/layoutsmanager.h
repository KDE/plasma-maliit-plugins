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


#ifndef LAYOUTSMANAGER_H
#define LAYOUTSMANAGER_H

#include "keyboarddata.h"
#include "keyboardmapping.h"
#include "layoutdata.h"
#include "mvirtualkeyboardstyle.h"

#include <MGConfItem>
#include <MLocale>
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
    static void createInstance(const MVirtualKeyboardStyleContainer *styleContainer);

    //! \brief Destroy singleton
    static void destroyInstance();

    //! \brief Returns the count of keyboard layouts.
    int layoutCount() const;

    //! \brief Returns the keyboard layout file name list.
    QStringList layoutFileList() const;

    //! Returns title of a given \a layoutFile as stated in xml.
    QString keyboardTitle(const QString &layoutFile) const;

    //! Returns whether autocaps is enabled for given \a layoutFile.
    bool autoCapsEnabled(const QString &layoutFile) const;

    //! \return keyboard language (as specified in XML) of a given \a layoutFile.
    QString keyboardLanguage(const QString &layoutFile) const;

    //! \brief Get layout model by layoutFile, type, and orientation
    const LayoutData *layout(const QString &layoutFile, LayoutData::LayoutType type,
                             M::Orientation orientation) const;

    //! \brief Get layout model specific to current hardware keyboard layout.
    const LayoutData *hardwareLayout(LayoutData::LayoutType type,
                                     M::Orientation orientation) const;

    //! \brief Returns currently set default layout file name.
    QString defaultLayoutFile() const;

    //! \brief Returns currently system display language
    QString systemDisplayLanguage() const;

    //! \brief Returns the model for hardware keyboard
    QString xkbModel() const;

    //! \brief Returns current layout for hardware keyboard
    QString xkbLayout() const;

    //! \brief Returns current variant for hardware keyboard
    QString xkbVariant() const;

    //! \brief Returns the primary layout for hardware keyboard
    QString xkbPrimaryLayout() const;

    //! \brief Returns the primary variant for hardware keyboard
    QString xkbPrimaryVariant() const;

    //! \brief Returns the secondary layout for hardware keyboard
    QString xkbSecondaryLayout() const;

    //! \brief Returns the secondary variant for hardware keyboard
    QString xkbSecondaryVariant() const;

    //! \brief Sets \a layout and \a variant as the xkb map for hardware keyboard.
    void setXkbMap(const QString &layout, const QString &variant);

    //! \brief Returns whether autocaps is enabled for hardware keyboard.
    bool hardwareKeyboardAutoCapsEnabled() const;

    /*!
     *\brief Returns the layout file names and their titles for selected keyboards.
     */
    QMap<QString, QString> selectedLayouts() const;

signals:
    //! Signals that layouts have been reset and keyboard data can
    //! be reloaded using new layouts returned by layoutList().
    void layoutsChanged();

    //! Signals that number format have been reset and number/phonenumber
    //! keyboard data can be reloaded.
    void numberFormatChanged();

    //! Signals that selected layouts have been changed.
    void selectedLayoutsChanged();

    //! The xkb layout of hardware keyboard has been changed.
    void hardwareLayoutChanged();

private:
    //! Default constructor
    explicit LayoutsManager(const MVirtualKeyboardStyleContainer *styleContainer = 0);

    /*!
     * Returns the layout of the given layout name
     * \param layout layout name such as fi_FI.xml or fi.xml
     * \return true if layout was loaded, false if failure or layout already existed
     */
    bool loadLayout(const QString &layout);

    //! \return keyboard that matches layout list entry /a layoutFile
    const KeyboardData *keyboardByName(const QString &layoutFile) const;

    //! Maps HW Sym variant type to xml file which contains the symbols.
    QString symbolVariantFileName(HardwareSymbolVariant symVariant);

    //! Initialize the xkb keyboard map.
    void initXkbMap();

private slots:
    void syncLayouts();
    void syncHardwareKeyboard();
    void syncNumberKeyboards();

private:
    //! Valid states of number format setting
    enum NumberFormat {
        NumArabic,
        NumLatin
    };

    //! MGConfItem for selected layouts available for vkb's use.
    //! The settings are set by control panel applet.
    MGConfItem configLayouts;

    //! Setting that tells the xkb model.
    MGConfItem xkbModelSetting;

    //! Current xkb layout.
    QString xkbCurrentLayout;

    //! Current xkb variant.
    QString xkbCurrentVariant;

    //! All keyboards arranged by layout name.
    //! The map key is layout name as read from settings.
    QMap<QString, KeyboardData *> keyboards;

    //! Iterator that points to current layout and keyboard.
    //! Current keyboard is used when no layout name is specified
    //! in calls to public methods of this class.
    QMap<QString, KeyboardData *>::const_iterator current;

    //! The shared style container.
    const MVirtualKeyboardStyleContainer *const styleContainer;

    //! Current hardware layout (xkb) specific keyboard.
    KeyboardData hwKeyboard;

    //! Current number keyboard
    KeyboardData numberKeyboard;

    //! Current phone number keyboard
    KeyboardData phoneNumberKeyboard;

    //! Setting that determines whether number format is Arabic or Latin
    MGConfItem numberFormatSetting;

    //! Current state of number format setting (represented by \a numberFormatSetting)
    NumberFormat numberFormat;

    //! System locale
    MLocale locale;

    HardwareKeyboardLayout currentHwkbLayoutType;


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
