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

#include "mkeyboardsettings.h"
#include "mkeyboardsettingswidget.h"
#include "keyboarddata.h"

#include <QObject>
#include <QGraphicsWidget>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace {
    const QString SettingsIMCorrectionSetting("/meegotouch/inputmethods/correctionenabled");
    const QString InputMethodLanguages("/meegotouch/inputmethods/languages");
    const QString VKBConfigurationPath("/usr/share/meegotouch/virtual-keyboard/layouts/");
    const QString VKBLayoutsFilterRule("*.xml");
    const QString VKBLayoutsIgnoreRules("number|test"); // use as regexp to ignore number and test layouts
};

MKeyboardSettings::MKeyboardSettings()
    : keyboardErrorCorrectionConf(SettingsIMCorrectionSetting, this),
      selectedKeyboardsConf(InputMethodLanguages, this)
{
    readAvailableKeyboards();
    connect(&keyboardErrorCorrectionConf, SIGNAL(valueChanged()),
            this, SIGNAL(errorCorrectionChanged()));
    connect(&selectedKeyboardsConf, SIGNAL(valueChanged()),
            this, SIGNAL(selectedKeyboardsChanged()));
}

MKeyboardSettings::~MKeyboardSettings()
{
}

QGraphicsWidget *MKeyboardSettings::createContentWidget(QGraphicsWidget *parent)
{
    // the pointer of returned QGraphicsWidget is owned by the caller,
    // so we just always create a new containerWidget.
    return new MKeyboardSettingsWidget(this, parent);
}

QString MKeyboardSettings::title()
{
    //% "Virtual keyboards";
    return qtTrId("qtn_txts_virtual_keyboards");;
}

QString MKeyboardSettings::icon()
{
    return "";
}

void MKeyboardSettings::readAvailableKeyboards()
{
    availableKeyboardInfos.clear();
    // available keyboard languages are determined by xml layouts that can be found
    const QDir layoutsDir(VKBConfigurationPath, VKBLayoutsFilterRule);
    QRegExp ignoreExp(VKBLayoutsIgnoreRules, Qt::CaseInsensitive);

    foreach (QFileInfo keyboardFileInfo, layoutsDir.entryInfoList()) {
        if (keyboardFileInfo.fileName().contains(ignoreExp))
            continue;
        KeyboardData keyboard;
        if (keyboard.loadNokiaKeyboard(keyboardFileInfo.fileName())) {
            if (keyboard.language().isEmpty() || keyboard.title().isEmpty())
                continue;
            bool duplicated = false;
            foreach (const KeyboardInfo &info, availableKeyboardInfos) {
                if (info.language == keyboard.language()) {
                    duplicated = true;
                    break;
                }
            }
            if (!duplicated) {
                KeyboardInfo keyboardInfo;
                keyboardInfo.fileName = keyboardFileInfo.fileName();
                keyboardInfo.language = keyboard.language();
                keyboardInfo.title = keyboard.title();
                availableKeyboardInfos.append(keyboardInfo);
            }
        }
    }
}

QMap<QString, QString> MKeyboardSettings::availableKeyboards() const
{
    QMap<QString, QString> keyboards;
    foreach (const KeyboardInfo &keyboardInfo, availableKeyboardInfos) {
        keyboards.insert(keyboardInfo.language, keyboardInfo.title);
    }
    return keyboards;
}

QMap<QString, QString> MKeyboardSettings::selectedKeyboards() const
{
    QMap<QString, QString> keyboards;
    foreach (const QString language, selectedKeyboardsConf.value().toStringList()) {
        keyboards.insert(language, keyboardTitle(language));
    }
    return keyboards;
}

void MKeyboardSettings::setSelectedKeyboards(const QStringList &keyboardTitles)
{
    QStringList languages;
    foreach (const QString &title, keyboardTitles) {
        QString language = keyboardLanguage(title);
        if (!language.isEmpty() && !languages.contains(language)) {
            languages.append(language);
        }
    }
    selectedKeyboardsConf.set(languages);
}

QString MKeyboardSettings::keyboardTitle(const QString &language) const
{
    QString title;
    foreach (const KeyboardInfo &keyboardInfo, availableKeyboardInfos) {
        if (keyboardInfo.language == language) {
            title = keyboardInfo.title;
            break;
        }
    }
    return title;
}

QString MKeyboardSettings::keyboardLanguage(const QString &title) const
{
    QString language;
    foreach (const KeyboardInfo &keyboardInfo, availableKeyboardInfos) {
        if (keyboardInfo.title == title) {
            language = keyboardInfo.language;
            break;
        }
    }
    return language;
}

bool  MKeyboardSettings::errorCorrection() const
{
    return keyboardErrorCorrectionConf.value().toBool();
}

void  MKeyboardSettings::setErrorCorrection(bool enabled)
{
    if (keyboardErrorCorrectionConf.value().toBool() != enabled)
        keyboardErrorCorrectionConf.set(enabled);
}
