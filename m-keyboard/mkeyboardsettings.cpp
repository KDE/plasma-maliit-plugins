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
    const QString SettingsFuzzy("/meegotouch/inputmethods/virtualkeyboard/fuzzyselected");
    const QString SettingsWordPrediction("/meegotouch/inputmethods/virtualkeyboard/wordpredictionenabled");
    const QString SettingsScriptPriorityZh("/meegotouch/inputmethods/virtualkeyboard/scriptpriority/zh");
};

MKeyboardSettings::MKeyboardSettings()
    : keyboardErrorCorrectionConf(SettingsImErrorCorrection),
      keyboardCorrectionSpaceConf(SettingsImCorrectionSpace),
      selectedKeyboardsConf(InputMethodLayouts),
      chineseKeyboardFuzzyConf(SettingsFuzzy),
      chineseKeyboardWordPredictionConf(SettingsWordPrediction),
      chineseKeyboardScriptPriorityConf(SettingsScriptPriorityZh)
{
    readAvailableKeyboards();
    connect(&keyboardErrorCorrectionConf, SIGNAL(valueChanged()),
            this, SIGNAL(errorCorrectionChanged()));
    connect(&keyboardCorrectionSpaceConf, SIGNAL(valueChanged()),
            this, SIGNAL(correctionSpaceChanged()));
    connect(&selectedKeyboardsConf, SIGNAL(valueChanged()),
            this, SIGNAL(selectedKeyboardsChanged()));

    connect(&chineseKeyboardFuzzyConf, SIGNAL(valueChanged()),
            this, SIGNAL(fuzzyChanged()));
    connect(&chineseKeyboardWordPredictionConf, SIGNAL(valueChanged()),
            this, SIGNAL(wordPredictionChanged()));
    connect(&chineseKeyboardScriptPriorityConf, SIGNAL(valueChanged()),
            this, SIGNAL(simpTradPriorityChanged()));
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

bool MKeyboardSettings::fuzzyPinyin() const
{
    return chineseKeyboardFuzzyConf.value().toBool();
}

void MKeyboardSettings::setFuzzyPinyin(bool enabled)
{
    if (chineseKeyboardFuzzyConf.value().toBool() != enabled)
         chineseKeyboardFuzzyConf.set(enabled);
}

bool MKeyboardSettings::wordPrediction() const
{
    return chineseKeyboardWordPredictionConf.value().toBool();
}

void MKeyboardSettings::setWordPrediction(bool enabled)
{
    if (chineseKeyboardWordPredictionConf.value().toBool() != enabled)
         chineseKeyboardWordPredictionConf.set(enabled);
}

bool MKeyboardSettings::scriptPriority() const
{
    return chineseKeyboardScriptPriorityConf.value().toBool();
}

void MKeyboardSettings::setScriptPriority(bool value)
{
    if (chineseKeyboardScriptPriorityConf.value().toBool() != value)
         chineseKeyboardScriptPriorityConf.set(value);
}
