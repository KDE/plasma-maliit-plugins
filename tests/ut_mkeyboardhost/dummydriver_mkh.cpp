/* This file is part of meegotouch-inputmethodkeyboard *
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

#include "dummydriver_mkh.h"

DummyDriverMkh::DummyDriverMkh()
    : predictionEnabledFlag(false),
      correctionEnabledFlag(false),
      completionEnabledFlag(false),
      maximumErrorsCount(0),
      maximumCandidatesCount(0),
      suggestedCandidateIndexValue(0)
{
}

DummyDriverMkh::~DummyDriverMkh()
{
}

bool DummyDriverMkh::init()
{
    return true;
}

bool DummyDriverMkh::addDictionaryWord(const QString &w, MImEngine::DictionaryType type)
{
    Q_UNUSED(w);
    Q_UNUSED(type);
    return false;
}

bool DummyDriverMkh::removeDictionaryWord(const QString &w, MImEngine::DictionaryType type)
{
    Q_UNUSED(w);
    Q_UNUSED(type);
    return false;
}

bool DummyDriverMkh::disableDictionary(MImEngine::DictionaryType type)
{
    Q_UNUSED(type);
    return false;
}
bool DummyDriverMkh::enableDictionary(MImEngine::DictionaryType type)
{
    Q_UNUSED(type);
    return false;
}
bool DummyDriverMkh::removeDictionary(MImEngine::DictionaryType type)
{
    Q_UNUSED(type);
    return false;
}

void DummyDriverMkh::appendString(const QString &s)
{
    Q_UNUSED(s);
}

void DummyDriverMkh::reselectString(const QString &s)
{
    Q_UNUSED(s);
}

void DummyDriverMkh::appendCharacter(const QChar &c)
{
    Q_UNUSED(c);
}

void DummyDriverMkh::tapKeyboard(const QPoint &position, bool shift, QChar symbol)
{
    Q_UNUSED(position);
    Q_UNUSED(shift);
    Q_UNUSED(symbol);
}

void DummyDriverMkh::setContext(const QString &s, int cursor)
{
    Q_UNUSED(s);
    Q_UNUSED(cursor);
}

void DummyDriverMkh::disablePrediction()
{
    predictionEnabledFlag = false;
}

void DummyDriverMkh::enablePrediction()
{
    predictionEnabledFlag = true;
}

bool DummyDriverMkh::predictionEnabled()
{
    return predictionEnabledFlag;
}

void DummyDriverMkh::disableCorrection()
{
    correctionEnabledFlag = false;
}

void DummyDriverMkh::enableCorrection()
{
    correctionEnabledFlag = true;
}

bool DummyDriverMkh::correctionEnabled()
{
    return correctionEnabledFlag;
}

void DummyDriverMkh::disableCompletion()
{
    completionEnabledFlag = false;
}

void DummyDriverMkh::enableCompletion()
{
    completionEnabledFlag = true;
}

bool DummyDriverMkh::completionEnabled()
{
    return completionEnabledFlag;
}

QStringList DummyDriverMkh::candidates(unsigned int uStartIndex, unsigned int uNum)
{
    Q_UNUSED(uStartIndex);
    Q_UNUSED(uNum);
    return candidateList;
}

int DummyDriverMkh::totalCandidates()
{
    return 0;
}

int DummyDriverMkh::matchedLength()
{
    return 0;
}

int DummyDriverMkh::suggestedCandidateIndex()
{
    return suggestedCandidateIndexValue;
}

bool  DummyDriverMkh::setSuggestedCandidateIndex(int index)
{
    Q_UNUSED(index);
    return true;
}

void DummyDriverMkh::setExactWordPositionInList(MImEngine::ExactInListType setting)
{
    Q_UNUSED(setting);
}

MImEngine::DictionaryType DummyDriverMkh::candidateSource(int index)
{
    Q_UNUSED(index);
    return MImEngine::DictionaryTypeInvalid;
}

bool DummyDriverMkh::setKeyboardLayoutKeys(const QList<MImEngine::KeyboardLayoutKey> &keys)
{
    Q_UNUSED(keys);
    return true;
}

QList<MImEngine::KeyboardLayoutKey> DummyDriverMkh::keyboardLayoutKeys()
{
    QList<MImEngine::KeyboardLayoutKey> keys;
    return keys;
}

void DummyDriverMkh::clearEngineBuffer()
{
}

void DummyDriverMkh::saveAndClearEngineBuffer()
{
}

QString  DummyDriverMkh::language()
{
    return driverLanguage;
}

bool DummyDriverMkh::setLanguage(const QString  &l, MImEngine::LanguagePriority p)
{
    Q_UNUSED(p);
    driverLanguage = l;
    return true;
}

bool DummyDriverMkh::setKeyboardLayout(const QString &l)
{
    kdbLanguage = l;
    return true;
}

QString DummyDriverMkh::keyboardLayout()
{
    return kdbLanguage;
}

bool DummyDriverMkh::exportAsNokiaDictionary(const QString &fileName)
{
    Q_UNUSED(fileName);
    return false;
}

bool DummyDriverMkh::importNokiaUserDictionary(const QString &fileName)
{
    Q_UNUSED(fileName);
    return false;
}

void DummyDriverMkh::setMaximumErrors(int maxErrors)
{
    maximumErrorsCount = maxErrors;
}

int DummyDriverMkh::maximumErrors()
{
    return maximumErrorsCount;
}

QString DummyDriverMkh::driver()
{
    return "dummyimdriver_mkh";
}

void DummyDriverMkh::setMaximumCandidates(int maxCandidates)
{
    maximumCandidatesCount = maxCandidates;
}

int DummyDriverMkh::maximumCandidates()
{
    return maximumCandidatesCount;
}

QString DummyDriverMkh::error()
{
    return QString();
}

void DummyDriverMkh::setCandidates(const QStringList &candidates)
{
    candidateList = candidates;
}

void DummyDriverMkh::setSuggestedCandidateIndexReturnValue(int index)
{
    suggestedCandidateIndexValue = index;
}
