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

#include "mabstractinputmethodhost.h"
#include "enginemanager.h"
#include "abstractengine.h"
#include "enginedefault.h"
#include "enginecjk.h"
#include "abstractenginewidgethost.h"
#include "mimcorrectionhost.h"
#include "sharedhandlearea.h"
#include "wordribbonhost.h"
#include "cjklogicstatemachine.h"
#include "mvirtualkeyboard.h"
#include "symbolview.h"
#include "keyevent.h"
#include "enginehandlerdefault.h"
#include "enginehandlertonal.h"

#include "mkeyboardhost.h"
#include <mimenginefactory.h>
#include <MGConfItem>
#include <MSceneWindow>


#include <QDebug>

namespace
{
    const QString DefaultInputLanguage("default");
    const QString InputMethodCorrectionEngineRoot("/meegotouch/inputmethods/correctionengine");
}

class EngineHandlerCJK : public EngineHandler
{
public:
    EngineHandlerCJK(MKeyboardHost &keyboardHost)
        : EngineHandler(keyboardHost),
          mKeyboardHost(keyboardHost),
          mEngineWidgetHost(new WordRibbonHost(keyboardHost.sceneWindow, 0)),
          stateMachine(NULL)
    {
        mEngineWidgetHost->watchOnWidget(keyboardHost.vkbWidget);
        mEngineWidgetHost->watchOnWidget(keyboardHost.symbolView);
        keyboardHost.sharedHandleArea->watchOnWidget(mEngineWidgetHost->inlineWidget());
    }

    virtual ~EngineHandlerCJK()
    {
        delete mEngineWidgetHost;
        mEngineWidgetHost = 0;

        // Don't forget to destroy CJK logic state machine here.
        if (stateMachine != NULL) {
            delete stateMachine;
            stateMachine = NULL;
        }
    }

    static QStringList supportedLanguages()
    {
        QStringList languages;
        languages << "zh" << "jp"; // ko will be handled in EngineHandlerKorean
        return languages;
    }

    //! \reimp
    virtual void activate()
    {
        connect(mEngineWidgetHost,
                SIGNAL(candidateClicked(const QString &, int)),
                &mKeyboardHost,
                SLOT(handleCandidateClicked(const QString &, int)),
                Qt::UniqueConnection);
        mEngineWidgetHost->finalizeOrientationChange();
        mEngineWidgetHost->showEngineWidget(AbstractEngineWidgetHost::DockedMode);

        // Create CJK logic state machine only when the engine is ready.
        if (stateMachine == NULL
            && (mKeyboardHost.inputMethodHost() != NULL)
            && (EngineManager::instance().engine() != NULL)) {
            stateMachine = new CJKLogicStateMachine(*mEngineWidgetHost,
                                                    *(mKeyboardHost.inputMethodHost()),
                                                    *(EngineManager::instance().engine()));
            stateMachine->setSyllableDivideEnabled(false);
        }

        if (stateMachine != NULL) {
            connect(stateMachine,   SIGNAL(toggleKeyStateChanged(bool)),
                    &mKeyboardHost, SLOT(handleToggleKeyStateChanged(bool)),
                    Qt::UniqueConnection);

            connect(stateMachine,   SIGNAL(composeStateChanged(bool)),
                    &mKeyboardHost, SLOT(handleComposeKeyStateChanged(bool)),
                    Qt::UniqueConnection);
        }
    }

    virtual void deactivate()
    {
        disconnect(mEngineWidgetHost, 0,
                   &mKeyboardHost,    0);

        if (stateMachine != NULL)
            disconnect(stateMachine,  0,
                       &mKeyboardHost, 0);
    }

    virtual AbstractEngineWidgetHost *engineWidgetHost()
    {
        return mEngineWidgetHost;
    }

    virtual bool cursorCanMoveInsidePreedit() const
    {
        return false;
    }

    virtual bool hasHwKeyboardIndicator() const
    {
        return true;
    }

    virtual bool hasErrorCorrection() const
    {
        return true;
    }

    virtual bool acceptPreeditInjection() const
    {
        return false;
    }

    virtual bool hasAutoCaps() const
    {
        return false;
    }

    virtual QList<QRegExp> autoCapsTriggers() const
    {
        return QList<QRegExp>();
    }

    virtual bool hasContext() const
    {
        return false;
    }

    virtual bool commitPreeditWhenInterrupted() const
    {
        return true;
    }

    bool correctionAcceptedWithSpaceEnabled() const
    {
        return false;
    }

    virtual bool isComposingInputMethod() const
    {
        return true;
    }

    virtual bool supportTouchPointAccuracy() const
    {
        return false;
    }

    virtual bool commitWhenCandidateClicked() const
    {
        return false;
    }

    virtual void clearPreedit(bool commit)
    {
        Q_UNUSED(commit);
    }

    virtual void editingInterrupted()
    {
        if (stateMachine != NULL)
            stateMachine->resetWithCommitStringToApp();
    }

    virtual void resetHandler()
    {
        if (stateMachine != NULL)
            stateMachine->resetWithoutCommitStringToApp();
    }

    virtual void preparePluginSwitching()
    {
    }

    virtual bool handleKeyPress(const KeyEvent &event)
    {
        if (stateMachine != NULL)
            return stateMachine->handleKeyPress(event);
        else
            return false;
    }

    virtual bool handleKeyRelease(const KeyEvent &event)
    {
        if (stateMachine != NULL)
            return stateMachine->handleKeyRelease(event);
        else
            return false;
    }

    virtual bool handleKeyClick(const KeyEvent &event, bool cycleKeyActive)
    {
        if (stateMachine != NULL)
            return stateMachine->handleKeyClick(event, cycleKeyActive);
        else
            return false;
    }

    virtual bool handleKeyCancel(const KeyEvent &event)
    {
        if (stateMachine != NULL)
            return stateMachine->handleKeyCancel(event);
        else
            return false;
    }
    //! \reimp_end

private:
    MKeyboardHost &mKeyboardHost;
    AbstractEngineWidgetHost *mEngineWidgetHost;
    CJKLogicStateMachine *stateMachine;
};

/*
 * Korean (Hangul) input engine handler class
 *
 * One Korean character is a composition of multiple Korean alphabets.
 * Each single Korean character has its unique sequence of typings.
 * Korean input system needs to compose the sequence into a character
 * and display the procedure of the composition.
 * ex. "ㅎ", "ㅏ" and "ㄴ" becomes "한". Users see "ㅎ", "하" and "한" 
 * in the preedit area as they type.
 *
 * Hangul input engine produces commit and preedit characters 
 * in every key input. The engine handler should take those in every input 
 * and show them properly.
 *
 * The handler assume that Hangul engine produce a single candidate with
 * a commit character and a preedit character. The commit charater is 
 * available when a Korean character is composed completely.
 * So if the length of candidate string is 1, this means the candidate string 
 * is preedit string. If the length of candidate is 2, the first character is 
 * commit and the last one is preedit.
 *
 * EngineHandlerKorean is based on the most famous Hangul engine: libhangul.
 *
 */
class EngineHandlerKorean : public EngineHandler
{
public:
    EngineHandlerKorean(MKeyboardHost &keyboardHost)
        : EngineHandler(keyboardHost),
          mKeyboardHost(keyboardHost),
          mEngineWidgetHost(new MImCorrectionHost(keyboardHost.sceneWindow, 0))
    {
        engine = EngineManager::instance().engine();
    }
    
    virtual ~EngineHandlerKorean()
    {
    }
    
    static QStringList supportedLanguages()
    {
        QStringList languages;
        languages << "ko";
        return languages;
    }

    //! \reimp
    virtual void activate()
    {
        connect(mEngineWidgetHost,
            SIGNAL(candidateClicked(const QString &, int)), &mKeyboardHost,
            SLOT(handleCandidateClicked(const QString &, int)), 
            Qt::UniqueConnection);
        mEngineWidgetHost->finalizeOrientationChange();
    }

    virtual void deactivate()
    {
        disconnect(mEngineWidgetHost, 0, &mKeyboardHost,    0);
    }

    virtual AbstractEngineWidgetHost *engineWidgetHost()
    {
        // return default error correction host
        return mEngineWidgetHost;
    }

    virtual bool cursorCanMoveInsidePreedit() const
    {
        return false;
    }

    virtual bool hasHwKeyboardIndicator() const
    {
        return true;
    }

    virtual bool hasErrorCorrection() const
    {
        return true;
    }

    virtual bool acceptPreeditInjection() const
    {
        return true;
    }

    virtual bool hasAutoCaps() const
    {
        return false;
    }

    virtual QList<QRegExp> autoCapsTriggers() const
    {
        return QList<QRegExp>();
    }

    virtual bool hasContext() const
    {
        return false;
    }

    virtual bool commitPreeditWhenInterrupted() const
    {
        return true;
    }

    virtual bool correctionAcceptedWithSpaceEnabled() const
    {
        return MGConfItem(CorrectionSettingWithSpace)
            .value(DefaultCorrectionSettingAcceptedWithSpaceOption).toBool();
    }

    virtual bool isComposingInputMethod() const
    {
        return false;
    }

    virtual bool supportTouchPointAccuracy() const
    {
        return true;
    }

    virtual bool commitWhenCandidateClicked() const
    {
        return true;
    }

    virtual void clearPreedit(bool commit)
    {
        if (!mKeyboardHost.preedit.isEmpty()) {
            if (commit) {
                // Commit current preedit
                mKeyboardHost.inputMethodHost()
                    ->sendCommitString(mKeyboardHost.preedit, 0, 0, 
                    mKeyboardHost.preeditCursorPos);
            } else {
                // Clear current preedit
                QList<MInputMethod::PreeditTextFormat> preeditFormats;
                MInputMethod::PreeditTextFormat preeditFormat(0, 0, 
                    MInputMethod::PreeditKeyPress);
                preeditFormats << preeditFormat;
                mKeyboardHost.inputMethodHost()->sendPreeditString("", 
                    preeditFormats);
            }
            mKeyboardHost.preedit.clear();
        }
    }

    virtual void editingInterrupted()
    {
        clearPreedit(commitPreeditWhenInterrupted());
    }

    virtual void resetHandler()
    {
    }

    virtual void preparePluginSwitching()
    {
    }

    virtual bool handleKeyPress(const KeyEvent &event)
    {
        Q_UNUSED(event);
        return false;
    }

    virtual bool handleKeyRelease(const KeyEvent &event)
    {
        Q_UNUSED(event);
        return false;
    }

    virtual bool handleKeyClick(const KeyEvent &event, bool cycleKeyActive)
    {
        QChar typedChar = event.text().at(0);
        KeyEvent::SpecialKey specialkey = event.specialKey();
        Qt::Key qtkey = event.qtKey();
        
        if ((specialkey == KeyEvent::LayoutMenu)
            || (specialkey == KeyEvent::Sym) 
            || (specialkey == KeyEvent::OnOffToggle)
            || (qtkey == Qt::Key_Return)
            || (specialkey == KeyEvent::Commit)
            || (qtkey == Qt::Key_Space)) {
            flushOut();
            return false;
        } else if (qtkey == Qt::Key_Backspace) {
            // Hangul engine cares backspace when there is something in preedit.
            if (getPreedit().length() == 0) {
              return false;
            } else {
              typedChar = '\b';
            }
        } else if (typedChar.toAscii() != 0) {
            flushOut();
            return false;
        }
        
        engine->appendCharacter(typedChar);
        
        const QString commitString = getCommit();
        const QString preeditString = getPreedit();
        
        if (!commitString.isEmpty()) {
            sendCommit(commitString);
            engine->setSuggestedCandidateIndex(0); // always choose the first one
        }
        sendPreedit(preeditString);
        
        return true;
    }

    virtual bool handleKeyCancel(const KeyEvent &event)
    {
        Q_UNUSED(event);
        return false;
    }

    //! \reimp_end

private:
    void sendPreedit(const QString & str)
    {
        QList < MInputMethod::PreeditTextFormat > preeditFormats;
        MInputMethod::PreeditTextFormat preeditFormat(0, str.length(), 
            MInputMethod::PreeditKeyPress);
        preeditFormats << preeditFormat;
        // preedit length is always 1
        mKeyboardHost.inputMethodHost()->sendPreeditString(str, preeditFormats,
            0, 0, -1);
    }
    
    void sendCommit(const QString & str)
    {
        if (str.length() > 0)
            mKeyboardHost.inputMethodHost()->sendCommitString(str, 0, 0, -1);
        // send commited signal.
        // Hangul engine will clear commit string but keep preedit string
        engine->setSuggestedCandidateIndex(0);
    }
    
    QString getPreedit()
    {
        QStringList list;
        list = engine->candidates(0, 0);
        if (!list.isEmpty()) {
            QString str = list.at(0);
            // preedit character is the last 1 character of the only candidate
            return str.right(1);
        } else {
            return QString();
        }
    }
    
    QString getCommit()
    {
        QStringList list;
        list = engine->candidates(0, 0);
        if (!list.isEmpty()) {
            QString str = list.at(0);
            // commit character is the first characters if any
            return str.length() <= 1 ? QString() : str.left(str.length() - 1);
        } else {
            return QString();
        }
    }

    void flushOut() 
    {
        QString str = getCommit();
        str.append(getPreedit());
        sendCommit(str);
        clearCandidates();
    }
    
    void clearCandidates()
    {
        engine->clearEngineBuffer();
    }
private:
    MKeyboardHost &mKeyboardHost;
    AbstractEngineWidgetHost *mEngineWidgetHost;
    MImEngineWordsInterface *engine;
};

EngineManager *EngineManager::Instance = 0;

EngineManager::EngineManager(MKeyboardHost &keyboardHost)
    : QObject(),
      mKeyboardHost(keyboardHost),
      currentEngineHandler(0),
      currentEngine(0)
{
}

EngineManager::~EngineManager()
{
    qDeleteAll(handlerMap.values());
    qDeleteAll(engineMap.values());
}

void EngineManager::createInstance(MKeyboardHost &keyboardHost)
{
    Q_ASSERT(!Instance);
    if (!Instance) {
        Instance = new EngineManager(keyboardHost);
        Instance->init();
    }
}

void EngineManager::destroyInstance()
{
    Q_ASSERT(Instance);
    delete Instance;
    Instance = 0;
}

EngineManager &EngineManager::EngineManager::instance()
{
    Q_ASSERT(Instance);
    return *Instance;
}

EngineHandler *EngineManager::handler() const
{
    return currentEngineHandler;
}

EngineHandler *EngineManager::handler(const QString &lang)
{
    if (lang == mLanguage)
        return currentEngineHandler;

    const QString language = lang.contains("@") ? lang.split('@').first()
                                   : lang;

    return findOrCreateEngineHandler(language);
}

MImEngineWordsInterface *EngineManager::engine() const
{
    if (currentEngine)
        return currentEngine->engine();
    else
        return 0;
}

void EngineManager::init()
{
    // initialize the language properties map
    handlerMap.insert(DefaultInputLanguage,
                                 new EngineHandlerDefault(mKeyboardHost));

    // initialize the engine map
    MGConfItem defaultEngineSetting(InputMethodCorrectionEngineRoot);
    if (!defaultEngineSetting.value().isNull()) {
        engineMap.insert(DefaultInputLanguage,
                         new EngineDefault(*(mKeyboardHost.inputMethodHost()),
                                             defaultEngineSetting.value().toString()));
    }
    mLanguage = DefaultInputLanguage;
}

QString EngineManager::activeLanguage() const
{
    return mLanguage;
}

void EngineManager::ensureLanguageInUse(const QString &lang)
{
    if (!languageIsValid()) {
        mLanguage.clear();
        updateLanguage(lang);
    }
}

bool EngineManager::languageIsValid() const
{
    if (!currentEngine || !currentEngine->engine())
        return false;

    const QString cachedEngineLang = mLanguage.contains("@") ? mLanguage.split('@').last()
                                                             : mLanguage;
    const QString actualEngineLang = currentEngine->engine()->language();

    return (cachedEngineLang == actualEngineLang);
}

void EngineManager::updateLanguage(const QString &lang)
{
    qDebug() << __PRETTY_FUNCTION__ << "- used language:" << lang;
    if (lang == mLanguage)
        return;

    const QString language = lang.contains("@") ? lang.split('@').first()
                                   : lang;

    AbstractEngine *matchedEngine = 0;
    MGConfItem settingEngineByLang(InputMethodCorrectionEngineRoot + "/" + language);
    if (!settingEngineByLang.value().isNull()) {
        // find and create engine if it not exists in the map.
        matchedEngine = findOrCreateEngine(language, settingEngineByLang.value().toString());
    } else {
        // use default engine, note: it could be also NULL
        matchedEngine = engineMap.value(DefaultInputLanguage).data();
    }

    if (currentEngine != matchedEngine 
        && currentEngine != 0
        && currentEngineHandler != 0
        && currentEngineHandler->engineWidgetHost()) {
        currentEngineHandler->engineWidgetHost()->hideEngineWidget();
    }

    if (currentEngineHandler) {
        if (currentEngineHandler->engineWidgetHost())
            currentEngineHandler->engineWidgetHost()->reset();

        currentEngineHandler->editingInterrupted();
    }

    currentEngine = matchedEngine;
    if (currentEngine) {
        currentEngine->updateEngineLanguage(lang);
    }

    EngineHandler *oldEngineHandler = currentEngineHandler;

    currentEngineHandler = findOrCreateEngineHandler(language);

    if(oldEngineHandler!= currentEngineHandler) {
        if (oldEngineHandler)
            oldEngineHandler->deactivate();

        if (currentEngineHandler)
            currentEngineHandler->activate();
    }

    mLanguage = lang;
}

EngineHandler *EngineManager::findOrCreateEngineHandler(const QString &language)
{
    qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language;
    QMap<QString, QPointer<EngineHandler> >::iterator matchedIt = handlerMap.find(language);
    QPointer<EngineHandler> matchedEngineHandler = 0;

    if (matchedIt != handlerMap.end()) {
        // already in the map
        matchedEngineHandler = matchedIt.value().data();

    } else if (EngineHandlerCJK::supportedLanguages().contains(language)){
        // create CJK language properties
        matchedEngineHandler = QPointer<EngineHandlerCJK>(new EngineHandlerCJK(mKeyboardHost));
        foreach (const QString &lang, EngineHandlerCJK::supportedLanguages())
            handlerMap.insert(lang, matchedEngineHandler);

    } else if (EngineHandlerEnglish::supportedLanguages().contains(language)){
        matchedEngineHandler = QPointer<EngineHandlerEnglish>(new EngineHandlerEnglish(mKeyboardHost));
        foreach (const QString &lang, EngineHandlerEnglish::supportedLanguages())
            handlerMap.insert(lang, matchedEngineHandler);
    } else if (EngineHandlerTonal::supportedLanguages().contains(language)){
        matchedEngineHandler = QPointer<EngineHandlerTonal>(new EngineHandlerTonal(mKeyboardHost));
        foreach (const QString &lang, EngineHandlerTonal::supportedLanguages())
            handlerMap.insert(lang, matchedEngineHandler);
    } else if (EngineHandlerKorean::supportedLanguages().contains(language)) {
        matchedEngineHandler = QPointer<EngineHandlerKorean>(new EngineHandlerKorean(mKeyboardHost));
        foreach (const QString &lang, EngineHandlerKorean::supportedLanguages())
            handlerMap.insert(lang, matchedEngineHandler);
    } else {
        // use default language properties
        matchedEngineHandler = handlerMap.value(DefaultInputLanguage).data();
    }

    return matchedEngineHandler;
}

AbstractEngine *EngineManager::findOrCreateEngine(const QString &language,
                                                                       const QString &engineName)
{
    qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language << "," << engineName;
    QMap<QString, QPointer<AbstractEngine> >::iterator matchedIt = engineMap.find(language);
    QPointer<AbstractEngine> matchedEngine = 0;

    if (matchedIt != engineMap.end()) {
        // the engine for language is already in the map
        matchedEngine = matchedIt.value().data();
    } else if (EngineCJK::supportedLanguages().contains(language)){
        // create CJK engine
        matchedEngine = QPointer<EngineCJK>(
                new EngineCJK(*(mKeyboardHost.inputMethodHost()), engineName));

        foreach (const QString &lang, EngineCJK::supportedLanguages())
            engineMap.insert(lang, matchedEngine);
    } else if (EngineKorean::supportedLanguages().contains(language)) {
        matchedEngine = QPointer<EngineKorean>(
	        new EngineKorean(*(mKeyboardHost.inputMethodHost()), engineName));
        foreach (const QString &lang, EngineKorean::supportedLanguages())
            engineMap.insert(lang, matchedEngine);
    } else {
        // use default engine
        matchedEngine = engineMap.value(DefaultInputLanguage).data();
    }

    return matchedEngine;
}
