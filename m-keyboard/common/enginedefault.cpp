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

#include "enginedefault.h"
#include "enginemanager.h"

#include <mimenginefactory.h>
#include <MGConfItem>
#include <QDebug>

namespace
{
    const QString DefaultInputLanguage("en_GB");
    const QString CorrectionSetting("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    const QString CorrectionSettingWithSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
    const QString InputMethodNextWordPredictionSetting("/meegotouch/inputmethods/virtualkeyboard/nextwordpredictionenabled");
    const bool DefaultCorrectionSettingOption = true;
    const bool DefaultCorrectionSettingAcceptedWithSpaceOption = false;
    const int MaximumErrorCorrectionCandidate = 5;
};

EngineDefault::EngineDefault(MAbstractInputMethodHost &imHost, const QString &engineName)
    : AbstractEngine(imHost, engineName),
      inputMethodHost(imHost),
      mEngine(MImEngineFactory::instance()->createEngineWords(engineName)),
      settingCorrection(new MGConfItem(CorrectionSetting)),
      settingCorrectionSpace(new MGConfItem(CorrectionSettingWithSpace)),
      settingNextWordPrediction(new MGConfItem(InputMethodNextWordPredictionSetting))
{
    if (mEngine) {
        initializeEngine();
    } else {
        qWarning() << __PRETTY_FUNCTION__ << "Failed to load correction default engine";
    }
}

EngineDefault::~EngineDefault()
{
    if (mEngine)
        MImEngineFactory::instance()->deleteEngine(mEngine);
}

MImEngineWordsInterface *EngineDefault::engine() const
{
    return mEngine;
}

QStringList EngineDefault::supportedLanguages()
{
    return QStringList();
}

void EngineDefault::initializeEngine()
{
    if (!mEngine)
        return;
    updateEngineLanguage(DefaultInputLanguage);
    connect(settingCorrection, SIGNAL(valueChanged()),
            this,              SLOT(synchronizeCorrectionSetting()));

    connect(this,                                SIGNAL(correctionSettingChanged()),
            &EngineManager::instance(), SIGNAL(correctionSettingChanged()));

    connect(settingNextWordPrediction, SIGNAL(valueChanged()),
            this,                      SLOT(synchronizeNextWordPrediction()));
}

void EngineDefault::synchronizeCorrectionSetting()
{
    bool correction = settingCorrection->value(DefaultCorrectionSettingOption).toBool();

    if (!correction) {
        mEngine->disableCorrection();
        mEngine->disableCompletion();
    } else {
        mEngine->enableCorrection();
        mEngine->enableCompletion();
    }

    emit correctionSettingChanged();
}

void EngineDefault::synchronizeNextWordPrediction()
{
    if (settingNextWordPrediction->value().toBool())
        mEngine->enablePrediction();
    else
        mEngine->disablePrediction();
}

void EngineDefault::updateEngineLanguage(const QString &language)
{
    if (!mEngine)
        return;
    if (!language.isEmpty()) {
        qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language;

        // TODO: maybe we should check return values here and in case of failure
        // be always in accurate mode, for example
        mEngine->setLanguage(language, MImEngine::LanguagePriorityPrimary);
        synchronizeCorrectionSetting();
        synchronizeNextWordPrediction();
        mEngine->disablePrediction();
        mEngine->setMaximumCandidates(MaximumErrorCorrectionCandidate);
        mEngine->setExactWordPositionInList(MImEngine::ExactInListFirst);
    }
}

