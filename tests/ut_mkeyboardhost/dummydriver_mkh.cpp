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

QStringList DummyDriverMkh::matchedSyllables()
{
    return QStringList();
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
    if (index >= 0 && index < candidateSources.size())
        return candidateSources[index];

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

bool DummyDriverMkh::setScript(const QString &s)
{
    Q_UNUSED(s);
    return false;
}

QString DummyDriverMkh::script()
{
    return QString();
}

QString DummyDriverMkh::transliterate(const QString &targetScript, unsigned int index)
{
    Q_UNUSED(targetScript);
    Q_UNUSED(index);
    return QString();
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

void DummyDriverMkh::insertCharacters(const QString &text, int index)
{
    Q_UNUSED(text);
    Q_UNUSED(index);
}

void DummyDriverMkh::removeCharacters(int count, int index)
{
    Q_UNUSED(count);
    Q_UNUSED(index);
}

void DummyDriverMkh::setCandidates(const QStringList &candidates)
{
    candidateList = candidates;
}

void DummyDriverMkh::setCandidateSources(const QList<MImEngine::DictionaryType> &candidateSource)
{
    candidateSources = candidateSource;
}

void DummyDriverMkh::setSuggestedCandidateIndexReturnValue(int index)
{
    suggestedCandidateIndexValue = index;
}
