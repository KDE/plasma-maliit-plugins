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
#include "abstractenginewidgethost.h"
#include "keyevent.h"
#include "mkeyboardhost.h"
#include "enginehandlerdefault.h"
#include "mimcorrectionhost.h"

#include <MSceneWindow>
#include <MGConfItem>

namespace
{
    //! Unicode Object Replacement character
    const QChar ObjectReplacementChar(0xfffc);

    //! Unicode Right Double Quotation mark
    const QChar RightDoubleQuotationMark(0x201d);

    //! Unicode Double Low-9 Quotation mark
    const QChar DoubleLowQuotationMark(0x201e);

    const QRegExp AutoCapsTrigger(QString("[.?!¡¿%1] +$").arg(ObjectReplacementChar));
    const QRegExp AutoCapsQuotationTrigger(QString("[.?!](\"|'|%1|%2|\"'|'\"|%1'|'%1|%2'|'%2) +$").arg(RightDoubleQuotationMark).arg(DoubleLowQuotationMark));

    const bool DefaultCorrectionSettingAcceptedWithSpaceOption = false;
    const QString CorrectionSettingWithSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
}

EngineHandlerDefault::EngineHandlerDefault(MKeyboardHost &keyboardHost)
    : EngineHandler(keyboardHost),
      mKeyboardHost(keyboardHost),
      mEngineWidgetHost(new MImCorrectionHost(keyboardHost.sceneWindow, 0))
{
    autoCapsTriggerList << AutoCapsTrigger;
}

EngineHandlerDefault::~EngineHandlerDefault() {
    delete mEngineWidgetHost;
    mEngineWidgetHost = 0;
}

//! \reimp
void EngineHandlerDefault::activate()
{
    connect(mEngineWidgetHost,
            SIGNAL(candidateClicked(const QString &, int)),
            &mKeyboardHost,
            SLOT(handleCandidateClicked(const QString &, int)),
            Qt::UniqueConnection);
    mEngineWidgetHost->finalizeOrientationChange();
}

void EngineHandlerDefault::deactivate()
{
    disconnect(mEngineWidgetHost, 0,
               &mKeyboardHost,    0);
}

AbstractEngineWidgetHost* EngineHandlerDefault::engineWidgetHost()
{
    // return default error correction host
    return mEngineWidgetHost;
}

bool EngineHandlerDefault::cursorCanMoveInsidePreedit() const
{
    return true;
}

bool EngineHandlerDefault::hasHwKeyboardIndicator() const
{
    return true;
}

bool EngineHandlerDefault::hasErrorCorrection() const
{
    return true;
}

bool EngineHandlerDefault::acceptPreeditInjection() const
{
    return true;
}

bool EngineHandlerDefault::hasAutoCaps() const
{
    return true;
}

QList<QRegExp> EngineHandlerDefault::autoCapsTriggers() const
{
    return autoCapsTriggerList;
}

bool EngineHandlerDefault::hasContext() const
{
    return true;
}

bool EngineHandlerDefault::commitPreeditWhenInterrupted() const
{
    return true;
}

bool EngineHandlerDefault::correctionAcceptedWithSpaceEnabled() const
{
    return MGConfItem(CorrectionSettingWithSpace).value(DefaultCorrectionSettingAcceptedWithSpaceOption).toBool();
}

bool EngineHandlerDefault::isComposingInputMethod() const
{
    return false;
}

bool EngineHandlerDefault::supportTouchPointAccuracy() const
{
    return true;
}

bool EngineHandlerDefault::commitWhenCandidateClicked() const
{
    return true;
}

void EngineHandlerDefault::clearPreedit(bool commit)
{
    if (!mKeyboardHost.preedit.isEmpty()) {
        if (commit) {
            // Commit current preedit
            mKeyboardHost.inputMethodHost()->sendCommitString(mKeyboardHost.preedit,
                                                              0, 0, mKeyboardHost.preeditCursorPos);
        } else {
            // Clear current preedit
            QList<MInputMethod::PreeditTextFormat> preeditFormats;
            MInputMethod::PreeditTextFormat preeditFormat(0, 0, MInputMethod::PreeditKeyPress);
            preeditFormats << preeditFormat;
            mKeyboardHost.inputMethodHost()->sendPreeditString("", preeditFormats);
        }
        mKeyboardHost.preedit.clear();
    }
}

void EngineHandlerDefault::editingInterrupted()
{
    clearPreedit(commitPreeditWhenInterrupted());
}

void EngineHandlerDefault::resetHandler()
{
}

void EngineHandlerDefault::preparePluginSwitching()
{
}

bool EngineHandlerDefault::handleKeyPress(const KeyEvent &event)
{
    Q_UNUSED(event);
    return false;
}

bool EngineHandlerDefault::handleKeyRelease(const KeyEvent &event)
{
    Q_UNUSED(event);
    return false;
}

bool EngineHandlerDefault::handleKeyClick(const KeyEvent &event, bool cycleKeyActive)
{
    Q_UNUSED(event);
    Q_UNUSED(cycleKeyActive);
    return false;
}

bool EngineHandlerDefault::handleKeyCancel(const KeyEvent &event)
{
    Q_UNUSED(event);
    return false;
}

EngineHandlerEnglish::EngineHandlerEnglish(MKeyboardHost &keyboardHost)
    : EngineHandlerDefault(keyboardHost)
{
    autoCapsTriggerList << AutoCapsTrigger << AutoCapsQuotationTrigger;
}

EngineHandlerEnglish::~EngineHandlerEnglish() {
}

QList<QRegExp> EngineHandlerEnglish::autoCapsTriggers() const
{
    return autoCapsTriggerList;
}
