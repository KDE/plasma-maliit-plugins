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

#ifndef DUMMYDRIVER_MKH_H
#define DUMMYDRIVER_MKH_H

#include <mimenginetypes.h>
#include <mimenginewordsinterface.h>
#include <QString>
#include <QStringList>
#include <QObject>

class MImEngineWords;

//! Just like the dummy im engine driver from meegotouch-inputmethodengine
//! except that this has functionality specific to the ut_mkeyboardhost
class DummyDriverMkh: public QObject, public MImEngineWordsInterface
{
    Q_OBJECT
    Q_DISABLE_COPY(DummyDriverMkh)
    Q_INTERFACES(MImEngineWordsInterface)

public:
    /*!
     * DummyDriverMkh construtor
     */
    DummyDriverMkh();

    /*!
     * DummyDriverMkh destructor
     */
    ~DummyDriverMkh();

    /*!
     *\reimp
    */

    virtual bool init();

    virtual bool addDictionaryWord(const QString &word, MImEngine::DictionaryType);

    virtual bool removeDictionaryWord(const QString &word, MImEngine::DictionaryType);

    virtual bool disableDictionary(MImEngine::DictionaryType);

    virtual bool enableDictionary(MImEngine::DictionaryType);

    virtual bool removeDictionary(MImEngine::DictionaryType);

    virtual void appendString(const QString &s);

    virtual void reselectString(const QString &s);

    virtual void appendCharacter(const QChar &c);

    virtual void tapKeyboard(const QPoint &position, bool shift, QChar symbol);

    virtual void setContext(const QString &s, int cursor);

    virtual void disablePrediction();

    virtual void enablePrediction();

    virtual bool predictionEnabled();

    virtual void disableCorrection();

    virtual void enableCorrection();

    virtual bool correctionEnabled();

    virtual void disableCompletion();

    virtual void enableCompletion();

    virtual bool completionEnabled();

    virtual QStringList candidates(unsigned int uStartIndex = 0, unsigned int uNum = 0);

    virtual int totalCandidates();

    virtual int matchedLength();

    virtual QStringList matchedSyllables();

    virtual int suggestedCandidateIndex();

    virtual bool setSuggestedCandidateIndex(int index);

    virtual void setExactWordPositionInList(MImEngine::ExactInListType setting);

    virtual MImEngine::DictionaryType candidateSource(int index);

    virtual bool setKeyboardLayoutKeys(const QList<MImEngine::KeyboardLayoutKey> &keys);

    virtual QList<MImEngine::KeyboardLayoutKey> keyboardLayoutKeys();

    virtual void clearEngineBuffer();

    virtual void saveAndClearEngineBuffer();

    virtual QString  language();

    virtual bool setLanguage(const QString &language, MImEngine::LanguagePriority);

    virtual bool setKeyboardLayout(const QString &);

    virtual QString keyboardLayout();

    virtual bool exportAsNokiaDictionary(const QString &);

    virtual bool importNokiaUserDictionary(const QString &);

    virtual void setMaximumErrors(int);

    virtual int maximumErrors();

    virtual void setMaximumCandidates(int maxCandidates);

    virtual int maximumCandidates();

    virtual bool setScript(const QString &s);

    virtual QString script();

    virtual QString transliterate(const QString &targetScript, unsigned int index);

    virtual QString driver();

    virtual QString error();

    virtual void insertCharacters(const QString &text, int index = -1);

    virtual void removeCharacters(int count = 1, int index = -1);

    /*!
     *\reimp_end
    */

    // Special setters for ut_mkeyboardhost..................................

    void setCandidates(const QStringList &candidates);
    void setCandidateSources(const QList<MImEngine::DictionaryType> &candidateSource);
    //! \brief Note that this is different from setSuggestedCandidateIndex
    void setSuggestedCandidateIndexReturnValue(int index);

private:
    bool predictionEnabledFlag;
    bool correctionEnabledFlag;
    bool completionEnabledFlag;

    QString driverLanguage;
    QString kdbLanguage;

    int maximumErrorsCount;
    int maximumCandidatesCount;

    QStringList candidateList;
    QList<MImEngine::DictionaryType> candidateSources;
    int suggestedCandidateIndexValue;
};

#endif //DUMMYDRIVER_H
