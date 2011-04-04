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

#include "enginecjk.h"

#include <mimenginefactory.h>
#include <MGConfItem>

#include <QDebug>

namespace
{
    const QString DefaultInputLanguage("zh@pinyin");
    const QString InputMethodFuzzySetting("/meegotouch/inputmethods/virtualkeyboard/fuzzyselected");
    const QString InputMethodWordPredictionSetting("/meegotouch/inputmethods/virtualkeyboard/wordpredictionenabled");
    const QString InputMethodScriptPrioritySetting("/meegotouch/inputmethods/virtualkeyboard/scriptpriority");
};

EngineCJK::EngineCJK(MAbstractInputMethodHost &imHost, const QString &engineName)
    : AbstractEngine(imHost, engineName),
      inputMethodHost(imHost),
      mEngine(MImEngineFactory::instance()->createEngineWords(engineName)),
      settingFuzzy(new MGConfItem(InputMethodFuzzySetting)),
      settingWordPrediction(new MGConfItem(InputMethodWordPredictionSetting)),
      settingScriptPriority(new MGConfItem(InputMethodScriptPrioritySetting))
{
    if (mEngine) {
        initializeEngine();
    } else {
        qWarning() << __PRETTY_FUNCTION__ << "Failed to load input engine for "
            << engineName;
    }
}

EngineCJK::~EngineCJK()
{
    if (mEngine)
        MImEngineFactory::instance()->deleteEngine(mEngine);
}

MImEngineWordsInterface *EngineCJK::engine() const
{
    return mEngine;
}

QStringList EngineCJK::supportedLanguages()
{
    QStringList languages;
    languages << "zh" << "ja" << "ko";
    return languages;
}

bool EngineCJK::correctionAcceptedWithSpaceEnabled() const
{
    // not supported setting
    return false;
}

void EngineCJK::initializeEngine()
{
    if (!mEngine)
        return;
    updateEngineLanguage(DefaultInputLanguage);

    connect(settingFuzzy, SIGNAL(valueChanged()),
            this,         SLOT(synchronizeFuzzy()));

    connect(settingWordPrediction, SIGNAL(valueChanged()),
            this,         SLOT(synchronizeWordPrediction()));

    connect(settingScriptPriority, SIGNAL(valueChanged()),
            this,                    SLOT(synchronizeScriptPriority()));
}

void EngineCJK::updateEngineLanguage(const QString &language)
{
    if (!mEngine)
        return;
    if (!language.isEmpty()) {
        qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language;

        const QString variant = language.contains("@") ? language.split('@').last()
                                : language;
        // TODO: maybe we should check return values here and in case of failure
        // be always in accurate mode, for example
        mEngine->setLanguage(variant, MImEngine::LanguagePriorityPrimary);
        synchronizeFuzzy();
        synchronizeWordPrediction();
        synchronizeScriptPriority();
    }
}

void EngineCJK::synchronizeFuzzy()
{
    if (settingFuzzy->value().toBool())
        mEngine->enableCorrection();
    else
        mEngine->disableCorrection();
}

void EngineCJK::synchronizeWordPrediction()
{
    if (settingWordPrediction->value().toBool())
        mEngine->enablePrediction();
    else
        mEngine->disablePrediction();
}

void EngineCJK::synchronizeScriptPriority()
{
     // The value is string. E.g, if current language is Chinese, the value is either
     // "Hans" (simplified Chinese words first) or "Hant" (traditional Chinese words first)
    if (!settingScriptPriority->value().toBool())
        mEngine->setScript("Hans");
    else
        mEngine->setScript("Hant");
}
