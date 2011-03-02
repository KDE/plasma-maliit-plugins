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
    const QString SettingsImErrorCorrection("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    const QString SettingsImCorrectionSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
    const QString InputMethodLayouts("/meegotouch/inputmethods/virtualkeyboard/layouts");
    const QString VKBConfigurationPath("/usr/share/meegotouch/virtual-keyboard/layouts/");
    const QString VKBLayoutsFilterRule("*.xml");
    const QString VKBLayoutsIgnoreRules("number|test|customer|default"); // use as regexp to ignore number, test, customer and default layouts
    const QString ChineseVKBConfigurationPath("/usr/share/meegotouch/virtual-keyboard/layouts/chinese");
};

MKeyboardSettings::MKeyboardSettings()
    : keyboardErrorCorrectionConf(SettingsImErrorCorrection),
      keyboardCorrectionSpaceConf(SettingsImCorrectionSpace),
      selectedKeyboardsConf(InputMethodLayouts)
{
    readAvailableKeyboards();
    connect(&keyboardErrorCorrectionConf, SIGNAL(valueChanged()),
            this, SIGNAL(errorCorrectionChanged()));
    connect(&keyboardCorrectionSpaceConf, SIGNAL(valueChanged()),
            this, SIGNAL(correctionSpaceChanged()));
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
    //% "Virtual keyboards"
    return qtTrId("qtn_txts_virtual_keyboards");;
}

QString MKeyboardSettings::icon()
{
    return "";
}

void MKeyboardSettings::readAvailableKeyboards()
{
    availableKeyboardInfos.clear();
    QList<QDir> dirs;
    dirs << QDir(VKBConfigurationPath, VKBLayoutsFilterRule);
    // TO BE REMOVED
    // Add Chinese input method layout directories here to allow our setting
    // could manage Chinese IM layouts. This is a workaround, will be removed later.
    dirs << QDir(ChineseVKBConfigurationPath, VKBLayoutsFilterRule);
    QRegExp ignoreExp(VKBLayoutsIgnoreRules, Qt::CaseInsensitive);

    foreach (const QDir &dir, dirs) {
        // available keyboard layouts are determined by xml layouts that can be found
        foreach (const QFileInfo &keyboardFileInfo, dir.entryInfoList()) {
            if (keyboardFileInfo.fileName().contains(ignoreExp))
                continue;
            KeyboardData keyboard;
            if (keyboard.loadNokiaKeyboard(keyboardFileInfo.filePath())) {
                if (keyboard.layoutFile().isEmpty()
                    || keyboard.language().isEmpty()
                    || keyboard.title().isEmpty())
                    continue;
                bool duplicated = false;
                foreach (const KeyboardInfo &info, availableKeyboardInfos) {
                    if (info.layoutFile == keyboard.layoutFile()
                        || info.title == keyboard.title()) {
                        // strip duplicated layout which has the same layout/title
                        duplicated = true;
                        break;
                    }
                }
                if (!duplicated) {
                    KeyboardInfo keyboardInfo;
                    // strip the path, only save the layout file name.
                    keyboardInfo.layoutFile = QFileInfo(keyboard.layoutFile()).fileName();
                    keyboardInfo.title = keyboard.title();
                    availableKeyboardInfos.append(keyboardInfo);
                }
            }
        }
    }
}

QMap<QString, QString> MKeyboardSettings::availableKeyboards() const
{
    QMap<QString, QString> keyboards;
    foreach (const KeyboardInfo &keyboardInfo, availableKeyboardInfos) {
        keyboards.insert(keyboardInfo.layoutFile, keyboardInfo.title);
    }
    return keyboards;
}

QMap<QString, QString> MKeyboardSettings::selectedKeyboards() const
{
    QMap<QString, QString> keyboards;
    foreach (const QString layoutFile, selectedKeyboardsConf.value().toStringList()) {
        keyboards.insert(layoutFile, keyboardTitle(layoutFile));
    }
    return keyboards;
}

void MKeyboardSettings::setSelectedKeyboards(const QStringList &keyboardLayouts)
{
    selectedKeyboardsConf.set(keyboardLayouts);
}

QString MKeyboardSettings::keyboardTitle(const QString &layoutFile) const
{
    QString title;
    foreach (const KeyboardInfo &keyboardInfo, availableKeyboardInfos) {
        if (keyboardInfo.layoutFile == layoutFile) {
            title = keyboardInfo.title;
            break;
        }
    }
    return title;
}

QString MKeyboardSettings::keyboardLayoutFile(const QString &title) const
{
    QString layoutFile;
    foreach (const KeyboardInfo &keyboardInfo, availableKeyboardInfos) {
        if (keyboardInfo.title == title) {
            layoutFile = keyboardInfo.layoutFile;
            break;
        }
    }
    return layoutFile;
}

bool MKeyboardSettings::errorCorrection() const
{
    return keyboardErrorCorrectionConf.value().toBool();
}

void MKeyboardSettings::setErrorCorrection(bool enabled)
{
    keyboardErrorCorrectionConf.set(enabled);
}

bool MKeyboardSettings::correctionSpace() const
{
    return keyboardCorrectionSpaceConf.value().toBool();
}

void MKeyboardSettings::setCorrectionSpace(bool enabled)
{
    keyboardCorrectionSpaceConf.set(enabled);
}
