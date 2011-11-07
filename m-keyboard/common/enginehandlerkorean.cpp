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

#include "enginehandlerkorean.h"
#include "mabstractinputmethodhost.h"
#include "enginemanager.h"
#include "mimcorrectionhost.h"
#include "mkeyboardhost.h"
#include <mimenginefactory.h>
#include <MGConfItem>
#include <MSceneWindow>

namespace
{
    const bool DefaultCorrectionSettingAcceptedWithSpaceOption = false;
    const QString CorrectionSettingWithSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
}

EngineHandlerKorean::EngineHandlerKorean(MKeyboardHost &keyboardHost)
    : EngineHandlerDefault(keyboardHost),
      mKeyboardHost(keyboardHost),
      mEngineWidgetHost(new MImCorrectionHost(keyboardHost.sceneWindow, 0))
{
    engine = EngineManager::instance().engine();
}
 
EngineHandlerKorean::~EngineHandlerKorean()
{
}
    
QStringList EngineHandlerKorean::supportedLanguages()
{
    QStringList languages;
    languages << "ko";
    return languages;
}

//! \reimp
void EngineHandlerKorean::activate()
{
    connect(mEngineWidgetHost,
        SIGNAL(candidateClicked(const QString &, int)), &mKeyboardHost,
        SLOT(handleCandidateClicked(const QString &, int)), 
        Qt::UniqueConnection);
    mEngineWidgetHost->finalizeOrientationChange();
}

void EngineHandlerKorean::deactivate()
{
    disconnect(mEngineWidgetHost, 0, &mKeyboardHost,    0);
}

AbstractEngineWidgetHost *EngineHandlerKorean::engineWidgetHost()
{
    // return default error correction host
    return mEngineWidgetHost;
}

bool EngineHandlerKorean::cursorCanMoveInsidePreedit() const
{
    return false;
}

bool EngineHandlerKorean::hasHwKeyboardIndicator() const
{
    return true;
}

bool EngineHandlerKorean::hasErrorCorrection() const
{
    return true;
}

bool EngineHandlerKorean::acceptPreeditInjection() const
{
    return true;
}

bool EngineHandlerKorean::hasAutoCaps() const
{
    return false;
}

QList<QRegExp> EngineHandlerKorean::autoCapsTriggers() const
{
    return QList<QRegExp>();
}

bool EngineHandlerKorean::hasContext() const
{
    return false;
}

bool EngineHandlerKorean::commitPreeditWhenInterrupted() const
{
    return true;
}

bool EngineHandlerKorean::correctionAcceptedWithSpaceEnabled() const
{
    return MGConfItem(CorrectionSettingWithSpace)
        .value(DefaultCorrectionSettingAcceptedWithSpaceOption).toBool();
}

bool EngineHandlerKorean::isComposingInputMethod() const
{
    return false;
}

bool EngineHandlerKorean::supportTouchPointAccuracy() const
{
    return true;
}

bool EngineHandlerKorean::commitWhenCandidateClicked() const
{
    return true;
}

void EngineHandlerKorean::clearPreedit(bool commit)
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

void EngineHandlerKorean::editingInterrupted()
{
    clearPreedit(commitPreeditWhenInterrupted());
}

void EngineHandlerKorean::resetHandler()
{
}

void EngineHandlerKorean::preparePluginSwitching()
{
}

bool EngineHandlerKorean::handleKeyPress(const KeyEvent &event)
{
    Q_UNUSED(event);
    return false;
}

bool EngineHandlerKorean::handleKeyRelease(const KeyEvent &event)
{
    Q_UNUSED(event);
    return false;
}

bool EngineHandlerKorean::handleKeyClick(const KeyEvent &event, bool cycleKeyActive)
{
    Q_UNUSED(cycleKeyActive);
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

bool EngineHandlerKorean::handleKeyCancel(const KeyEvent &event)
{
    Q_UNUSED(event);
    return false;
}

//! \reimp_end

void EngineHandlerKorean::sendPreedit(const QString & str)
{
    QList < MInputMethod::PreeditTextFormat > preeditFormats;
    MInputMethod::PreeditTextFormat preeditFormat(0, str.length(), 
        MInputMethod::PreeditKeyPress);
    preeditFormats << preeditFormat;
    // preedit length is always 1
    mKeyboardHost.inputMethodHost()->sendPreeditString(str, preeditFormats,
        0, 0, -1);
}

void EngineHandlerKorean::sendCommit(const QString & str)
{
    if (str.length() > 0)
        mKeyboardHost.inputMethodHost()->sendCommitString(str, 0, 0, -1);
    // send commited signal.
    // Hangul engine will clear commit string but keep preedit string
    engine->setSuggestedCandidateIndex(0);
}

QString EngineHandlerKorean::getPreedit()
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
    
QString EngineHandlerKorean::getCommit()
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

void EngineHandlerKorean::flushOut() 
{
    QString str = getCommit();
    str.append(getPreedit());
    sendCommit(str);
    clearCandidates();
}

void EngineHandlerKorean::clearCandidates()
{
    engine->clearEngineBuffer();
}
