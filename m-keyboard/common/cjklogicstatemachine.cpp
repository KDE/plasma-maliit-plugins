/* * This file is part of meego-keyboard-zh *
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

#include "cjklogicstatemachine.h"
#include <mimenginewordsinterface.h>
#include <mabstractinputmethodhost.h>
#include "mimtoolbar.h"
#include "abstractenginewidgethost.h"
#include "layoutsmanager.h"
#include <QString>
#include <QDebug>
#include <QtCore>
#include <QObject>
#include <cctype>

#include <MTimestamp>

namespace {
    const int AutoBackspaceDelay = 500;      // in ms
    const int InitialCandidateCount = 20;
    const int BackspaceRepeatInterval = 100; // in ms

    const QString PinyinLang("pinyin");
    const QString ZhuyinLang("zhuyin");
    const QString CangjieLang("cangjie");
    const QString StandardCangjie("cangjie");
    const QString SuchengCangjie("cangjie:advanced");
    const int CangjieInputLimit = 5;

    const QString StandByStateString("standby_state");
    const QString MatchStateString("match_state");
    const QString PredictionStateString("prediction_state");
}

CJKLogicStateMachine::CJKLogicStateMachine(AbstractEngineWidgetHost &widgetHost,
                                                       MAbstractInputMethodHost &host,
                                                       MImEngineWordsInterface &engine)
    : currentState(NULL),
      standbyState(new StandbyState(this)),
      matchState(new MatchState(this)),
      predictionState(new PredictionState(this)),
      engineWidgetHost(widgetHost),
      inputMethodHost(host),
      inputMethodEngine(engine),
      backspaceTimer(new QTimer(this)),
      backspaceLongPressTriggered(false),
      syllableDivideIsEnabled(false),
      currentOnOffState(false)
{
    changeState(StandByStateString);

    backspaceTimer->setSingleShot(true);
    connect(backspaceTimer, SIGNAL(timeout()),
            this, SLOT(handleLongPressBackspaceKey()));

    connect(&engineWidgetHost, SIGNAL(candidateClicked(QString,int)),
            this, SLOT(handleCandidateClicked(const QString &, int)));
}


CJKLogicStateMachine::~CJKLogicStateMachine()
{
    if (standbyState != NULL) {
        delete standbyState;
        standbyState = NULL;
    }

    if (matchState != NULL) {
        delete matchState;
        matchState = NULL;
    }

    if (predictionState != NULL) {
        delete predictionState;
        predictionState = NULL;
    }

    currentState = NULL;

    backspaceTimer->stop();
    backspaceLongPressTriggered = false;

    syllableDivideIsEnabled = false;
}

bool CJKLogicStateMachine::handleKeyPress(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Backspace)
        return handleKeyEvent(event);
    else
        return false;
}

bool CJKLogicStateMachine::handleKeyRelease(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Backspace)
        return handleKeyEvent(event);
    else
        return false;
}

bool CJKLogicStateMachine::handleKeyClick(const KeyEvent &event)
{
    if (event.qtKey() != Qt::Key_Backspace)
        return handleKeyEvent(event);
    else
        return false;
}

bool CJKLogicStateMachine::handleKeyEvent(const KeyEvent &event)
{    
    mTimestamp("handleKeyEvent", "start");
    qDebug()<<Q_FUNC_INFO;
    bool val = false;
    if (event.specialKey() == KeyEvent::LayoutMenu) {
        handleLayoutMenuKey(event);
        val = true;
    } else if (event.specialKey() == KeyEvent::Sym) {
        handleSymbolKey(event);
        val = false; // Return false here to make upper layer switch to Symbol view.
    } else if (event.specialKey() == KeyEvent::OnOffToggle) {
        handleToggleKeyClicked();
        val = true;
    } else if (event.qtKey() == Qt::Key_Backspace) {
        handleBackspaceKey(event);
        val = true;
    } else if (event.qtKey() == Qt::Key_Return || event.specialKey() == KeyEvent::Commit) {
        handleEnterKey(event);
        val = true;
    } else if (event.qtKey() == Qt::Key_Space) {
        handleSpaceKey(event);
        val = true;
    }else if (event.toQKeyEvent().text().length() == 1) {
        QChar tmpChar = event.text().at(0);
        if (tmpChar.isDigit()) {
            handleDigitKey(event);
            val = true;
        } else if (tmpChar.isPrint()
                 && (!tmpChar.isPunct())
                 && (!tmpChar.isSymbol())) {
            handleLetterKey(event);
            val = true;
        } else if (tmpChar == '\'') {
            handleQuotationMarkKey(event);
            val = true;
        } else {
            handleOtherKey(event);
            val = true;
        }

    } else {        
        if (event.qtKey() >= Qt::Key_Left && event.qtKey()<= Qt::Key_Down) {
            handleArrowKey(event);
            val = true;
        }
        else
            qDebug() <<"Warning (in CJKLogicStateMachine::handleKeyEvent): Key event is not handled.";
    }
    mTimestamp("handleKeyEvent", "start");
    return val;
}

void CJKLogicStateMachine::handleOrientationChange(M::Orientation orientation)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleOrientationChange(orientation);
}

void CJKLogicStateMachine::resetWithCommitStringToApp()
{
    //Notify sub state to shutdown themselves.
    currentState->shutDown(true);

    //Clear itself.
    emit composeStateChanged(false);
    currentState = standbyState;
    currentState->initState();
    engineWidgetHost.reset();
    engineWidgetHost.hideEngineWidget();
    inputMethodEngine.clearEngineBuffer();
    backspaceTimer->stop();
    backspaceLongPressTriggered = false;

    syllableDivideIsEnabled = false;

    // Reset CangJie encoding type as standard Cangjie.
    if (inputMethodEngine.language() == CangjieLang)
        handleToggleKeyClicked(true);

    return ;
}

void CJKLogicStateMachine::resetWithoutCommitStringToApp()
{
    //Notify sub state to shutdown themselves.
    currentState->shutDown(false);

    //Clear itself.
    emit composeStateChanged(false);
    currentState = standbyState;
    currentState->initState();
    engineWidgetHost.reset();
    engineWidgetHost.hideEngineWidget();
    inputMethodEngine.clearEngineBuffer();
    backspaceTimer->stop();
    backspaceLongPressTriggered = false;

    syllableDivideIsEnabled = false;

    return ;
}

void CJKLogicStateMachine::setSyllableDivideEnabled(bool enabled)
{
    syllableDivideIsEnabled = enabled;
}

void CJKLogicStateMachine::handleLayoutMenuKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleLayoutMenuKey(event);
}

void CJKLogicStateMachine::handleSymbolKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleSymbolKey(event);
}

void CJKLogicStateMachine::handleSymbolSwitchKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    emit symbolSwitchKeyClicked();
}

void CJKLogicStateMachine::handleToggleKeyClicked(bool bReset)
{
    currentOnOffState = bReset ? false : (!currentOnOffState);
    currentState->handleToggleKeyClicked();
    inputMethodEngine.setLanguage(currentOnOffState ? SuchengCangjie : StandardCangjie,
                                  MImEngine::LanguagePriorityPrimary);
    emit toggleKeyStateChanged(currentOnOffState);
}

void CJKLogicStateMachine::handleDigitKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleDigitKey(event);
}

void CJKLogicStateMachine::handleLetterKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleLetterKey(event);
}

void CJKLogicStateMachine::handleQuotationMarkKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleQuotationMarkKey(event);
}

void CJKLogicStateMachine::handleSpaceKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleSpaceKey(event);
}

void CJKLogicStateMachine::handleBackspaceKey(const KeyEvent &event)
{
    if (event.type() == QEvent::KeyPress) {
        backspaceLongPressTriggered = false;
        backspaceTimer->start(AutoBackspaceDelay);
    } else if (event.type() == QEvent::KeyRelease) {
        backspaceTimer->stop();
        if (!backspaceLongPressTriggered) {
            currentState->handleBackspaceKey(event);
        }
    } else {
        qDebug() <<"Warning: Unknown backspace key type!";
    }

}

void CJKLogicStateMachine::handleLongPressBackspaceKey()
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleLongPressBackspaceKey();
    backspaceLongPressTriggered = true;
}

void CJKLogicStateMachine::handleEnterKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleEnterKey(event);
}

void CJKLogicStateMachine::handleOtherKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleOtherKey(event);
}

void CJKLogicStateMachine::handleArrowKey(const KeyEvent &event)
{
    currentState->handleArrowKey(event);
}

void CJKLogicStateMachine::handleArrowKeyEx(const KeyEvent &event)
{
    if (!engineWidgetHost.candidates().isEmpty()) {
        if (event.qtKey() == Qt::Key_Left )
            engineWidgetHost.handleNavigationKey(AbstractEngineWidgetHost::NaviKeyLeft);
        else if (event.qtKey() == Qt::Key_Right)
            engineWidgetHost.handleNavigationKey(AbstractEngineWidgetHost::NaviKeyRight);
        else if (event.qtKey() == Qt::Key_Up)
            engineWidgetHost.handleNavigationKey(AbstractEngineWidgetHost::NaviKeyUp);
        else if (event.qtKey() == Qt::Key_Down)
            engineWidgetHost.handleNavigationKey(AbstractEngineWidgetHost::NaviKeyDown);
    }
}

void CJKLogicStateMachine::sendPreedit(const QString &matchedPart, const QString &unmatchedPart)
{
    QString showingPreedit;

    //recognizedInputStringLength tell the UI which part can be recognized by engine. So the
    //part can't be recognized can be shown in red.
    int recognizedLength = 0;
    int unRecognizedLength = 0;

    //Add the matched part first. It doesn't matter if the matchedPart is empty.
    showingPreedit.append(matchedPart);
    recognizedLength += matchedPart.length();

    QString curLanguage = inputMethodEngine.language();
    //Add the unmatched part.
    if (curLanguage == CangjieLang || curLanguage == ZhuyinLang) {
        showingPreedit.append(unmatchedPart);
        recognizedLength = showingPreedit.length();
        unRecognizedLength = 0;
    } else if (curLanguage == PinyinLang && !syllableDivideIsEnabled) {
        showingPreedit.append(unmatchedPart);

        QStringList matchedList = inputMethodEngine.matchedSyllables();
        for (int i = 0; i < matchedList.count(); ++i) {
            QString temp = matchedList.at(i);
            recognizedLength += temp.length();
        }
        unRecognizedLength = showingPreedit.length() - recognizedLength;
    } else if (curLanguage == PinyinLang && syllableDivideIsEnabled) {
        //The different between recognizedInputStringLength and recognizedLength is that:
        //recognizedInputStringLength records the recognized length of the pure input preedit.
        //recognizedLength records the recognized length of the showing preedit.
        int recognizedInputStringLength = 0;
        QStringList matchedList = inputMethodEngine.matchedSyllables();

        for (int i = 0; i < matchedList.count(); ++i) {
            QString temp = matchedList.at(i);
            recognizedInputStringLength += temp.length();

            if (temp != "\'") {
                showingPreedit += temp;
                recognizedLength += temp.length();

                if (i + 1 < matchedList.count() && (matchedList.at(i + 1) != "\'")) {
                    showingPreedit += "\'";
                    recognizedLength += 1;
                }
            } else {
                showingPreedit += temp;
                recognizedLength += temp.length();
            }
        }

        //Find the obsolete string and append to the showing preedit.
        int obsoleteLength = unmatchedPart.length() - recognizedInputStringLength;
        if (obsoleteLength != 0) {
            QString obsoleteString = unmatchedPart.right(obsoleteLength);
            showingPreedit.append(obsoleteString);
        }

        unRecognizedLength = obsoleteLength;
    } else {
        qDebug() <<"Warning: Current Chinese engine language can't be recognized in state machine.";
        return ;
    }


    qDebug() << "CJKLogicStateMachine::sendPreedit unRecognizedLength = "
            << unRecognizedLength;

    MInputMethod::PreeditFace face = MInputMethod::PreeditDefault;
    QList<MInputMethod::PreeditTextFormat> preeditFormats;
    if (unRecognizedLength > 0) {
        MInputMethod::PreeditTextFormat preeditFormat(0, showingPreedit.length() - unRecognizedLength, face);
        preeditFormats << preeditFormat;

        MInputMethod::PreeditFace faceError = MInputMethod::PreeditNoCandidates;
        MInputMethod::PreeditTextFormat preeditFormatError(
                showingPreedit.length() - unRecognizedLength, unRecognizedLength, faceError);
        preeditFormats << preeditFormatError;
    }
    else {
        MInputMethod::PreeditTextFormat preeditFormat(0, showingPreedit.length(), face);
        preeditFormats << preeditFormat;
    }

    inputMethodHost.sendPreeditString(showingPreedit, preeditFormats, 0, 0,
                                      showingPreedit.length());
}


void CJKLogicStateMachine::handleCandidateClicked(const QString &candStr, int wordIndex)
{
    qDebug() <<Q_FUNC_INFO;
    currentState->handleCandidateClicked(candStr, wordIndex);
}

void CJKLogicStateMachine::emitLayoutMenuKeyClickSignal()
{
    emit layoutMenuKeyClicked();
}

void CJKLogicStateMachine::changeState(const QString &state)
{
    qDebug() <<"#### LogicStateMachine change state to "<<state;

    engineWidgetHost.reset();
    engineWidgetHost.hideEngineWidget();
    inputMethodEngine.clearEngineBuffer();

    if(currentState != NULL)
        currentState->clearState();

    if (state == StandByStateString) {
        currentState = standbyState;
        emit composeStateChanged(false);
    }
    else if (state == MatchStateString) {
        currentState = matchState;
        emit composeStateChanged(true);
    }
    else if (state == PredictionStateString) {
        currentState = predictionState;
        emit composeStateChanged(false);
    }
    else {
        qDebug() <<"Warning (in CJKLogicStateMachine::changeState) : No such state named : "
                 <<state;
    }

    if(currentState != NULL)
        currentState->initState();
    return ;
}



StandbyState::StandbyState(CJKLogicStateMachine * machine)
    : InputStateAbstract(machine)
{

}

StandbyState::~StandbyState()
{
}

void StandbyState::initState()
{
    qDebug() <<Q_FUNC_INFO;
    return ;
}

void StandbyState::clearState()
{
    return ;
}

void StandbyState::shutDown(bool needCommitString)
{
    Q_UNUSED(needCommitString);
    return ;
}

void StandbyState::handleOrientationChange(M::Orientation orientation)
{
    Q_UNUSED(orientation);
    return ;
}

void StandbyState::handleCandidateClicked(const QString &candStr, int wordIndex)
{
    Q_UNUSED(candStr);
    Q_UNUSED(wordIndex);
    return ;
}

void StandbyState::handleLayoutMenuKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->emitLayoutMenuKeyClickSignal();
    return ;
}

void StandbyState::handleSymbolKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    return ;
}

void StandbyState::handleDigitKey(const KeyEvent &event)
{
    if (event.type() == QEvent::KeyRelease) {
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyPress, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyRelease, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
    }

    return ;
}

void StandbyState::handleLetterKey(const KeyEvent &event)
{
    stateMachine->changeState(MatchStateString);
    stateMachine->handleLetterKey(event);
    return ;
}

void StandbyState::handleQuotationMarkKey(const KeyEvent &event)
{
    stateMachine->inputMethodHost.sendCommitString(event.toQKeyEvent().text());
    return ;
}

void StandbyState::handleSpaceKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->inputMethodHost.sendCommitString(QString(" "));
    return ;
}

void StandbyState::handleBackspaceKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    const KeyEvent tmpevent("\b", QEvent::KeyPress, Qt::Key_Backspace,
                            KeyEvent::NotSpecial,
                            Qt::NoModifier);
    stateMachine->inputMethodHost.sendKeyEvent(tmpevent.toQKeyEvent());
    return ;
}

void StandbyState::handleLongPressBackspaceKey()
{
    qDebug() <<Q_FUNC_INFO;
    stateMachine->backspaceTimer->start(BackspaceRepeatInterval);
    const KeyEvent tmpevent("\b", QEvent::KeyPress, Qt::Key_Backspace,
                            KeyEvent::NotSpecial,
                            Qt::NoModifier);
    stateMachine->inputMethodHost.sendKeyEvent(tmpevent.toQKeyEvent());
    return ;
}

void StandbyState::handleEnterKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->inputMethodHost.sendCommitString(event.toQKeyEvent().text());
    return ;
}

void StandbyState::handleOtherKey(const KeyEvent &event)
{
    qDebug()<<Q_FUNC_INFO
            <<" text = " <<event.toQKeyEvent().text()
            <<" type = " <<event.type()
            <<" modifiers = " <<event.modifiers();
    if (event.type() == QEvent::KeyRelease) {
        stateMachine->inputMethodHost.sendCommitString(event.toQKeyEvent().text());
    }

    return ;
}

void StandbyState::handleArrowKey(const KeyEvent &event)
{    
    const KeyEvent tmpevent("", QEvent::KeyPress, event.qtKey(),
                            KeyEvent::NotSpecial,
                            Qt::NoModifier);
    stateMachine->inputMethodHost.sendKeyEvent(tmpevent.toQKeyEvent());
}

void StandbyState::handleToggleKeyClicked()
{
    return;
}


MatchState::MatchState(CJKLogicStateMachine * machine)
    : InputStateAbstract(machine),
      currentMatchState(NULL),
      matchNotStartState(NULL),
      matchStartState(NULL),
      recognizedStringLength(0),
      matchIsTerminated(false)
{
    matchNotStartState = new MatchNotStartedState(this, machine);
    matchStartState = new MatchStartedState(this, machine);

    changeMatchState("match_not_start_state");
}

MatchState::~MatchState()
{
    currentMatchState = NULL;
    if (matchNotStartState != NULL) {
        delete matchNotStartState;
        matchNotStartState = NULL;
    }

    if (matchStartState != NULL) {
        delete matchStartState;
        matchStartState = NULL;
    }
}

void MatchState::initState()
{
    qDebug() <<Q_FUNC_INFO;
    inputPreedit.clear();
    recognizedStringLength = 0;
    matchIsTerminated = false;
    currentMatchState->initState();
}

void MatchState::clearState()
{
    qDebug() <<Q_FUNC_INFO;
    changeMatchState("match_not_start_state");
    inputPreedit.clear();
    recognizedStringLength = 0;
    matchIsTerminated = false;
}

void MatchState::shutDown(bool needCommitString)
{
    //Notify sub state to shutdown.
    currentMatchState->shutDown(needCommitString);

    //Clear the data inside this class.
    inputPreedit.clear();
    recognizedStringLength = 0;
    matchIsTerminated = false;

    //Set the sub state pointer to initial state.
    currentMatchState = matchNotStartState;
    currentMatchState->initState();
    return ;
}

void MatchState::handleOrientationChange(M::Orientation orientation)
{
    currentMatchState->handleOrientationChange(orientation);
}

void MatchState::handleCandidateClicked(const QString &candStr, int wordIndex)
{
    currentMatchState->handleCandidateClicked(candStr, wordIndex);
}


void MatchState::handleLayoutMenuKey(const KeyEvent &event)
{
    currentMatchState->handleLayoutMenuKey(event);
}

void MatchState::handleSymbolKey(const KeyEvent &event)
{
    currentMatchState->handleSymbolKey(event);
}

void MatchState::handleDigitKey(const KeyEvent &event)
{
    currentMatchState->handleDigitKey(event);
}

void MatchState::handleLetterKey(const KeyEvent &event)
{
    currentMatchState->handleLetterKey(event);
}

void MatchState::handleQuotationMarkKey(const KeyEvent &event)
{
    currentMatchState->handleQuotationMarkKey(event);
}

void MatchState::handleSpaceKey(const KeyEvent &event)
{
    currentMatchState->handleSpaceKey(event);
}

void MatchState::handleBackspaceKey(const KeyEvent &event)
{
    currentMatchState->handleBackspaceKey(event);
}

void MatchState::handleLongPressBackspaceKey()
{
    currentMatchState->handleLongPressBackspaceKey();
}

void MatchState::handleEnterKey(const KeyEvent &event)
{
    currentMatchState->handleEnterKey(event);
}

void MatchState::handleOtherKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    currentMatchState->handleOtherKey(event);
    return ;
}

void MatchState::handleArrowKey(const KeyEvent &event)
{     
    stateMachine->handleArrowKeyEx(event);
}

void MatchState::handleToggleKeyClicked()
{
    currentMatchState->handleToggleKeyClicked();
}


void MatchState::changeMatchState(QString newMatchState)
{
    qDebug() <<Q_FUNC_INFO;
    qDebug() <<"#### Match sub StateMachine change state to " <<newMatchState;

    if(currentMatchState != NULL)
        currentMatchState->clearState();

    if (newMatchState == "match_not_start_state")
        currentMatchState = matchNotStartState;
    else if (newMatchState == "match_start_state")
        currentMatchState = matchStartState;
    else {
        qDebug() <<"Warning (in MatchState::changeState) : No such state named : "
                 <<newMatchState;
    }

    if(currentMatchState != NULL)
        currentMatchState->initState();
    return ;
}


MatchNotStartedState::MatchNotStartedState(MatchState *matchMachine, CJKLogicStateMachine *machine)
    : InputStateAbstract(machine),
      matchStateMachine(matchMachine)
{
}

MatchNotStartedState::~MatchNotStartedState()
{

}

void MatchNotStartedState::initState()
{
    return ;
}

void MatchNotStartedState::clearState()
{
    return ;
}

void MatchNotStartedState::shutDown(bool needCommitString)
{
    if (needCommitString && matchStateMachine->inputPreedit != 0) {
        stateMachine->inputMethodHost.sendCommitString("");
    }
    return ;
}

void MatchNotStartedState::handleOrientationChange(M::Orientation orientation)
{
    Q_UNUSED(orientation);
    stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
    stateMachine->changeState(StandByStateString);
    return ;
}

void MatchNotStartedState::handleCandidateClicked(const QString &candStr, int wordIndex)
{
    if (matchStateMachine->matchIsTerminated)
        return ;
    matchStateMachine->changeMatchState("match_start_state");
    matchStateMachine->currentMatchState->handleCandidateClicked(candStr, wordIndex);
    return ;
}

void MatchNotStartedState::handleLayoutMenuKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->inputMethodHost.sendCommitString("");
    stateMachine->changeState(StandByStateString);
    stateMachine->emitLayoutMenuKeyClickSignal();
    return ;
}

void MatchNotStartedState::handleSymbolKey(const KeyEvent &event)
{
    Q_UNUSED(event);

    if (!matchStateMachine->matchIsTerminated) {
        matchStateMachine->changeMatchState("match_start_state");
        matchStateMachine->handleSymbolKey(event);
    } else {
        if (stateMachine->inputMethodEngine.language() == CangjieLang)
            stateMachine->inputMethodHost.sendCommitString("");
        else
            stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);

        stateMachine->changeState(StandByStateString);
    }
    return ;
}

void MatchNotStartedState::handleDigitKey(const KeyEvent &event)
{
    stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
    if (event.type() == QEvent::KeyRelease) {
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyPress, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyRelease, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
    }
    stateMachine->changeState(StandByStateString);
}

void MatchNotStartedState::handleLetterKey(const KeyEvent &event)
{
    qDebug() <<Q_FUNC_INFO;
    if (stateMachine->inputMethodEngine.language() == CangjieLang) {
        if (matchStateMachine->inputPreedit.count() >= CangjieInputLimit)
            return ;
    }
    matchStateMachine->inputPreedit += event.toQKeyEvent().text();

    stateMachine->inputMethodEngine.clearEngineBuffer();
    stateMachine->inputMethodEngine.appendString(matchStateMachine->inputPreedit);
    stateMachine->sendPreedit("", matchStateMachine->inputPreedit);

    QStringList tempCandidateWords = stateMachine->inputMethodEngine.candidates(0, InitialCandidateCount);
    qDebug() <<Q_FUNC_INFO <<" tmp candidate words count = " <<tempCandidateWords.count();
    if (tempCandidateWords.count() > 0) {
        matchStateMachine->matchIsTerminated = false;
        stateMachine->engineWidgetHost.setCandidates(tempCandidateWords);
        stateMachine->engineWidgetHost.setTitle(matchStateMachine->inputPreedit);
        stateMachine->engineWidgetHost.showEngineWidget(AbstractEngineWidgetHost::DockedMode);
    } else {
        matchStateMachine->matchIsTerminated = true;
        stateMachine->engineWidgetHost.reset();
        stateMachine->engineWidgetHost.hideEngineWidget();
    }
    return ;
}

void MatchNotStartedState::handleQuotationMarkKey(const KeyEvent &event)
{
    handleLetterKey(event);
    return ;
}

void MatchNotStartedState::handleSpaceKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    if (!stateMachine->engineWidgetHost.candidates().isEmpty()) {
        matchStateMachine->changeMatchState("match_start_state");
        matchStateMachine->currentMatchState->handleSpaceKey(event);
        return ;
    }

    if (stateMachine->inputMethodEngine.language() == CangjieLang)
        stateMachine->inputMethodHost.sendCommitString("");
    else
        stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
    stateMachine->changeState(StandByStateString);
}

void MatchNotStartedState::handleBackspaceKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    matchStateMachine->inputPreedit.chop(1);
    
    if (matchStateMachine->inputPreedit.isEmpty()) {
        stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
        stateMachine->changeState(StandByStateString);
    } else {
        stateMachine->inputMethodEngine.clearEngineBuffer();
        stateMachine->inputMethodEngine.appendString(matchStateMachine->inputPreedit);
        stateMachine->sendPreedit("", matchStateMachine->inputPreedit);

        QStringList tempCandidateWords = stateMachine->inputMethodEngine.candidates(0, InitialCandidateCount);
        if (tempCandidateWords.count() > 0) {
            matchStateMachine->matchIsTerminated = false;
            stateMachine->engineWidgetHost.setCandidates(tempCandidateWords);
            stateMachine->engineWidgetHost.setTitle(matchStateMachine->inputPreedit);
            stateMachine->engineWidgetHost.showEngineWidget(AbstractEngineWidgetHost::DockedMode);
        } else {
            matchStateMachine->matchIsTerminated = true;
            qDebug() <<"MatchNotStartedState::handleBackspaceKey()"
                     <<"-- Engine return nothing, match terminated(match none of pinyin str)";
        }
    }
}

void MatchNotStartedState::handleLongPressBackspaceKey()
{
    if (matchStateMachine->inputPreedit.size() != 0) {
        matchStateMachine->inputPreedit.clear();
        stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
        stateMachine->engineWidgetHost.reset();
        stateMachine->engineWidgetHost.hideEngineWidget();
        stateMachine->changeState(StandByStateString);
    }
}

void MatchNotStartedState::handleEnterKey(const KeyEvent &event)
{
    Q_UNUSED(event);

    if (stateMachine->inputMethodEngine.language() == CangjieLang) {
        handleSpaceKey(event);
        return ;
    }

    stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
    stateMachine->changeState(StandByStateString);
    return ;
}

void MatchNotStartedState::handleOtherKey(const KeyEvent &event)
{
    //handle punctuation key begin ...
    if (!event.toQKeyEvent().text().isEmpty()) {
        qDebug() << Q_FUNC_INFO << "text != Empty";
        QChar tmpChar = event.text().at(0);
        if (tmpChar.isPrint()) {
            qDebug() << Q_FUNC_INFO << "text is Print";
            if (!matchStateMachine->matchIsTerminated) {
                matchStateMachine->changeMatchState("match_start_state");
                matchStateMachine->handleOtherKey(event);
            } else {
                if (stateMachine->inputMethodEngine.language() == CangjieLang)
                    stateMachine->inputMethodHost.sendCommitString("");
                else
                    stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
                stateMachine->inputMethodHost.sendCommitString(tmpChar);
                stateMachine->changeState(StandByStateString);
            }
            return;
        }
    }
    //handle punctuation key end ...

    if(!matchStateMachine->inputPreedit.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "sendCommitString = " << matchStateMachine->inputPreedit;
        stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
    }
    if (event.type() == QEvent::KeyRelease) {
        qDebug() << Q_FUNC_INFO << "QEvent::KeyRelease sendKeyEvent"
                << "event.qtKey() =" << event.qtKey()
                << "event.modifiers()" << event.modifiers()
                << "event.text()" << event.text();
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyPress, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyRelease, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
    }
    //stateMachine->inputMethodHost.sendCommitString(event.toQKeyEvent().text());
    stateMachine->changeState(StandByStateString);
    return ;
}

void MatchNotStartedState::handleArrowKey(const KeyEvent &event)
{
   stateMachine->handleArrowKeyEx(event);
}

void MatchNotStartedState::handleToggleKeyClicked()
{
    stateMachine->inputMethodHost.sendCommitString("");
    stateMachine->changeState(StandByStateString);
}

MatchStartedState::MatchStartedState(MatchState * matchMachine, CJKLogicStateMachine * machine)
    : InputStateAbstract(machine),
      pinyinMatchedLength(0),
      matchString_startMatchPos(0),
      matchStateMachine(matchMachine)
{
}

MatchStartedState::~MatchStartedState()
{
}

void MatchStartedState::initState()
{
    matchString = matchStateMachine->inputPreedit;
    return ;
}

void MatchStartedState::clearState()
{
    pinyin_MatchedStepHistory.clear();
    matchString_MatchedStepHistory.clear();
    userChoosePhaseStepHistory.clear();

    pinyinMatchedLength = 0;
    matchString.clear();
    matchString_startMatchPos = 0;

    return ;
}

void MatchStartedState::shutDown(bool needCommitString)
{
    if (needCommitString && matchString != 0) {
        stateMachine->inputMethodHost.sendCommitString("");
    }

    pinyin_MatchedStepHistory.clear();
    matchString_MatchedStepHistory.clear();
    userChoosePhaseStepHistory.clear();

    pinyinMatchedLength = 0;
    matchString.clear();
    matchString_startMatchPos = 0;

    return ;
}

void MatchStartedState::handleOrientationChange(M::Orientation orientation)
{
    Q_UNUSED(orientation);
    stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
    stateMachine->changeState(StandByStateString);
}

void MatchStartedState::handleCandidateClicked(const QString &candStr, int wordIndex)
{
    qDebug() <<Q_FUNC_INFO;
    if (candStr.length() == 0 || wordIndex < 0) {
        qDebug() <<"Warning(MatchStartedState::handleCandidateClicked): Invalid user choose canidate index";
        stateMachine->userChoseCandidateString = "";
        return ;
    }
    stateMachine->userChoseCandidateString = candStr;
    userChoosePhaseStepHistory.append(stateMachine->userChoseCandidateString);
    stateMachine->inputMethodEngine.setSuggestedCandidateIndex(wordIndex);

    int matchLengthFromEngine = stateMachine->inputMethodEngine.matchedLength();
    pinyinMatchedLength += matchLengthFromEngine;
    qDebug() <<"Already matched pinyin letter: " <<pinyinMatchedLength;

    if (pinyinMatchedLength < matchStateMachine->inputPreedit.length()) {
        //Change matchedString according to user choose candidate.
        matchString.remove(matchString_startMatchPos, matchLengthFromEngine);
        matchString.insert(matchString_startMatchPos, stateMachine->userChoseCandidateString);
        matchString_startMatchPos += stateMachine->userChoseCandidateString.length();

        //Record pinyin match length and Chinese word length for backspace key later.
        pinyin_MatchedStepHistory.append(matchLengthFromEngine);
        matchString_MatchedStepHistory.append(stateMachine->userChoseCandidateString.length());

        QString queryPinyinStr = matchStateMachine->inputPreedit.mid(pinyinMatchedLength);

        stateMachine->inputMethodEngine.clearEngineBuffer();
        stateMachine->inputMethodEngine.appendString(queryPinyinStr);
        stateMachine->sendPreedit(matchString.left(matchString_startMatchPos),
                                  queryPinyinStr);

        QStringList tempCandidateWords = stateMachine->inputMethodEngine.candidates(0, InitialCandidateCount);
        if (tempCandidateWords.count() > 0) {
            matchStateMachine->matchIsTerminated = false;
            stateMachine->engineWidgetHost.setCandidates(tempCandidateWords);
            stateMachine->engineWidgetHost.setTitle(queryPinyinStr);
            stateMachine->engineWidgetHost.showEngineWidget(AbstractEngineWidgetHost::DockedMode);
            return;
        } else {
            matchStateMachine->matchIsTerminated = true;
            stateMachine->engineWidgetHost.reset();
            stateMachine->engineWidgetHost.hideEngineWidget();
        }
    } else if(pinyinMatchedLength == matchStateMachine->inputPreedit.length()) {
        qDebug() <<"pinyinMatchedLength == matchStateMachine->preedit.length()";
        matchString.remove(matchString_startMatchPos, matchLengthFromEngine);
        matchString.insert(matchString_startMatchPos, stateMachine->userChoseCandidateString);
        matchString_startMatchPos += stateMachine->userChoseCandidateString.length();

        stateMachine->inputMethodHost.sendCommitString(matchString);

        qDebug() <<"----------Time to learn a new phase";
        //Time to learn a new phase
        QString learnPhasePinyinString = matchStateMachine->inputPreedit;
        int insertPosition = 0;
        for(int i = 0; i < pinyin_MatchedStepHistory.size(); ++i) {
            insertPosition += pinyin_MatchedStepHistory.at(i);
            learnPhasePinyinString.insert(insertPosition, QString("@"));
            insertPosition += 1;
        }

        QString learnPhaseChineseString;
        for(int i = 0; i < userChoosePhaseStepHistory.size(); ++i) {
            learnPhaseChineseString.append(userChoosePhaseStepHistory.at(i));
            if (i != userChoosePhaseStepHistory.size() - 1)
                learnPhaseChineseString.append(QString("@"));
        }

        qDebug() <<"-------------Learn new phase"
                 <<"learnPhasePinyinString = " <<learnPhasePinyinString
                 <<"learnPhaseChineseString length = " <<learnPhaseChineseString.length();

        QString learnPhase = learnPhasePinyinString + "|" + learnPhaseChineseString;
        bool results = stateMachine->inputMethodEngine.addDictionaryWord(learnPhase, MImEngine::DictionaryTypeLanguage);
        qDebug() <<"----------Time to learn a new phase results = " <<results;

        if (stateMachine->inputMethodEngine.predictionEnabled())
            stateMachine->changeState(PredictionStateString);
        else
            stateMachine->changeState(StandByStateString);

    }
    return ;
}

void MatchStartedState::handleLayoutMenuKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->inputMethodHost.sendCommitString(matchString);
    stateMachine->emitLayoutMenuKeyClickSignal();
    return ;
}

void MatchStartedState::handleSymbolKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->engineWidgetHost.handleNavigationKey(AbstractEngineWidgetHost::NaviKeyOk);

    if(!this->matchString.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "sendCommitString = " << this->matchString;
        stateMachine->inputMethodHost.sendCommitString(this->matchString);
    }

    stateMachine->changeState(StandByStateString);
    return ;
}

void MatchStartedState::handleDigitKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->inputMethodHost.sendCommitString(matchString);
    if (event.type() == QEvent::KeyRelease) {
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyPress, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyRelease, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
    }
    stateMachine->changeState(StandByStateString);
    return ;
}

void MatchStartedState::handleLetterKey(const KeyEvent &event)
{
    Q_UNUSED(event);

    if (pinyinMatchedLength != 0) {
        matchStateMachine->inputPreedit += event.toQKeyEvent().text();
        matchString += event.toQKeyEvent().text();

        QString queryPinyinStr = matchStateMachine->inputPreedit.mid(pinyinMatchedLength);

        stateMachine->inputMethodEngine.clearEngineBuffer();
        stateMachine->inputMethodEngine.appendString(queryPinyinStr);
        stateMachine->sendPreedit(matchString.left(matchString_startMatchPos),
                                  queryPinyinStr);

        mTimestamp("handleLetterKey get candidates", "start");
        QStringList tempCandidateWords = stateMachine->inputMethodEngine.candidates(0, InitialCandidateCount);
        mTimestamp("handleLetterKey get candidates", "end");
        if (tempCandidateWords.count() > 0) {
            matchStateMachine->matchIsTerminated = false;
            stateMachine->engineWidgetHost.setCandidates(tempCandidateWords);
            stateMachine->engineWidgetHost.setTitle(queryPinyinStr);
            stateMachine->engineWidgetHost.showEngineWidget(AbstractEngineWidgetHost::DockedMode);
            return ;
        } else {
            matchStateMachine->matchIsTerminated = true;
            stateMachine->engineWidgetHost.reset();
            stateMachine->engineWidgetHost.hideEngineWidget();
            return ;
        }
    }
    return ;
}

void MatchStartedState::handleQuotationMarkKey(const KeyEvent &event)
{
    handleLetterKey(event);
}

void MatchStartedState::handleSpaceKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    if (matchStateMachine->matchIsTerminated) {
        stateMachine->inputMethodHost.sendCommitString(matchString);
        stateMachine->changeState(StandByStateString);
        return ;
    }

    if (!stateMachine->engineWidgetHost.candidates().isEmpty()) {
        stateMachine->engineWidgetHost.handleNavigationKey(AbstractEngineWidgetHost::NaviKeyOk);
    }
}

void MatchStartedState::handleBackspaceKey(const KeyEvent &event)
{
    Q_UNUSED(event);

    int pinyinBackStep = pinyin_MatchedStepHistory.last();
    pinyin_MatchedStepHistory.pop_back();

    int matchStringBackStep = matchString_MatchedStepHistory.last();
    matchString_MatchedStepHistory.pop_back();

    pinyinMatchedLength -= pinyinBackStep;
    QString pinyinPart = matchStateMachine->inputPreedit.mid(pinyinMatchedLength, pinyinBackStep);

    matchString_startMatchPos -= matchStringBackStep;
    matchString.remove(matchString_startMatchPos, matchStringBackStep);
    matchString.insert(matchString_startMatchPos, pinyinPart);

    matchStateMachine->matchIsTerminated = false;

    QString queryPinyinStr = matchStateMachine->inputPreedit.mid(pinyinMatchedLength);

    stateMachine->inputMethodEngine.clearEngineBuffer();
    stateMachine->inputMethodEngine.appendString(queryPinyinStr);
    stateMachine->sendPreedit(matchString.left(matchString_startMatchPos),
                              queryPinyinStr);

    QStringList tempCandidateWords = stateMachine->inputMethodEngine.candidates(0, InitialCandidateCount);
    if (tempCandidateWords.count() > 0) {
        stateMachine->engineWidgetHost.setCandidates(tempCandidateWords);
        stateMachine->engineWidgetHost.setTitle(queryPinyinStr);
        stateMachine->engineWidgetHost.showEngineWidget(AbstractEngineWidgetHost::DockedMode);
    } else {
        qDebug() <<"Warning: MatchStartedState::handleBackspaceKey";
        stateMachine->changeState(StandByStateString);
    }

    if (pinyinMatchedLength == 0) {
        matchStateMachine->changeMatchState("match_not_start_state");
    }

    return ;
}

void MatchStartedState::handleLongPressBackspaceKey()
{
    if (matchStateMachine->inputPreedit.size() != 0) {
        matchStateMachine->inputPreedit.clear();
        stateMachine->inputMethodHost.sendCommitString(matchStateMachine->inputPreedit);
        stateMachine->engineWidgetHost.reset();
        stateMachine->engineWidgetHost.hideEngineWidget();
        stateMachine->changeState(StandByStateString);
    }
}

void MatchStartedState::handleEnterKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->inputMethodHost.sendCommitString(matchString);
    stateMachine->changeState(StandByStateString);
    return ;
}

void MatchStartedState::handleOtherKey(const KeyEvent &event)
{
    //handle punctuation key begin ...
    if (!event.toQKeyEvent().text().isEmpty()) {
        qDebug() << Q_FUNC_INFO << "text != Empty";
        QChar tmpChar = event.text().at(0);
        if (tmpChar.isPrint()) {
            qDebug() << Q_FUNC_INFO << "text isPrint";
            if (!matchStateMachine->matchIsTerminated) {
                stateMachine->engineWidgetHost.handleNavigationKey(AbstractEngineWidgetHost::NaviKeyOk);
            }
            stateMachine->inputMethodHost.sendCommitString(this->matchString);
            stateMachine->inputMethodHost.sendCommitString(tmpChar);
            stateMachine->changeState(StandByStateString);
            return ;
        }
    }
    //handle punctuation key end ...

    if (event.type() == QEvent::KeyRelease) {
        qDebug() << Q_FUNC_INFO << "QEvent::KeyRelease sendKeyEvent"
                << "event.qtKey() =" << event.qtKey()
                << "event.modifiers()" << event.modifiers()
                << "event.text()" << event.text();
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyPress, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyRelease, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
    }
    //stateMachine->inputMethodHost.sendCommitString(event.toQKeyEvent().text());
    stateMachine->changeState(StandByStateString);
    return ;
}

void MatchStartedState::handleArrowKey(const KeyEvent &event)
{
   stateMachine->handleArrowKeyEx(event);
}

void MatchStartedState::handleToggleKeyClicked()
{
    stateMachine->inputMethodHost.sendCommitString("");
    stateMachine->changeState(StandByStateString);
}

PredictionState::PredictionState(CJKLogicStateMachine * machine)
    : InputStateAbstract(machine)
{

}

PredictionState::~PredictionState()
{
}

void PredictionState::initState()
{
    stateMachine->inputMethodEngine.clearEngineBuffer();
    stateMachine->inputMethodEngine.setContext(stateMachine->userChoseCandidateString, -1);
    QStringList tempPredictionWords = stateMachine->inputMethodEngine.candidates(0, InitialCandidateCount);
    if (tempPredictionWords.count() > 0) {
        stateMachine->engineWidgetHost.setCandidates(tempPredictionWords);
        stateMachine->engineWidgetHost.setTitle(stateMachine->userChoseCandidateString);
        stateMachine->engineWidgetHost.showEngineWidget(AbstractEngineWidgetHost::DockedMode);
    } else {
        stateMachine->changeState(StandByStateString);
    }
    return ;
}

void PredictionState::clearState()
{
    return ;
}

void PredictionState::shutDown(bool needCommitString)
{
    Q_UNUSED(needCommitString);
    return ;
}

void PredictionState::handleOrientationChange(M::Orientation orientation)
{
    Q_UNUSED(orientation);
    stateMachine->changeState(StandByStateString);
}

void PredictionState::handleCandidateClicked(const QString &candStr, int wordIndex)
{
    if (candStr.length() == 0 || wordIndex < 0) {
        stateMachine->userChoseCandidateString = "";
        return ;
    }

    stateMachine->userChoseCandidateString = candStr;
    stateMachine->inputMethodHost.sendCommitString(candStr);

    stateMachine->inputMethodEngine.clearEngineBuffer();
    qDebug() <<"PredictionState::handleCandidateClicked : string length" <<candStr.length();
    stateMachine->inputMethodEngine.setContext(candStr, -1);
    QStringList tempPredictionWords = stateMachine->inputMethodEngine.candidates(0, InitialCandidateCount);

    qDebug() <<"PredictionState::handleCandidateClicked : engine results length = "
             <<tempPredictionWords.length();
    if (tempPredictionWords.count() > 0) {
        stateMachine->engineWidgetHost.setCandidates(tempPredictionWords);
        stateMachine->engineWidgetHost.setTitle(stateMachine->userChoseCandidateString);
        stateMachine->engineWidgetHost.showEngineWidget(AbstractEngineWidgetHost::DockedMode);
        return;
    } else {
        stateMachine->changeState(StandByStateString);
        return;
    }
}

void PredictionState::handleLayoutMenuKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->changeState(StandByStateString);
    stateMachine->emitLayoutMenuKeyClickSignal();
    return ;
}

void PredictionState::handleSymbolKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->changeState(StandByStateString);
    return ;
}

void PredictionState::handleDigitKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    if (event.type() == QEvent::KeyRelease) {
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyPress, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
        stateMachine->inputMethodHost.sendKeyEvent(
                QKeyEvent(QEvent::KeyRelease, event.qtKey(), event.modifiers(), event.text(),
                          false, 1), MInputMethod::EventRequestEventOnly);
    }
    stateMachine->changeState(StandByStateString);
    return ;
}

void PredictionState::handleLetterKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->changeState(StandByStateString);
    stateMachine->currentState->handleLetterKey(event);
    return ;
}

void PredictionState::handleQuotationMarkKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->changeState(StandByStateString);
    stateMachine->currentState->handleLetterKey(event);
    return ;
}

void PredictionState::handleSpaceKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    if (!stateMachine->engineWidgetHost.candidates().isEmpty()) {
        stateMachine->engineWidgetHost.handleNavigationKey(AbstractEngineWidgetHost::NaviKeyOk);
    }
}

void PredictionState::handleBackspaceKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->changeState(StandByStateString);
}

void PredictionState::handleLongPressBackspaceKey()
{
    stateMachine->changeState(StandByStateString);
}

void PredictionState::handleEnterKey(const KeyEvent &event)
{
    Q_UNUSED(event);
    stateMachine->changeState(StandByStateString);
    return ;
}

void PredictionState::handleOtherKey(const KeyEvent &event)
{
    stateMachine->changeState(StandByStateString);
    stateMachine->currentState->handleOtherKey(event);
    return ;
}

void PredictionState::handleArrowKey(const KeyEvent &event)
{
   stateMachine->handleArrowKeyEx(event);
}

void PredictionState::handleToggleKeyClicked()
{
    stateMachine->changeState(StandByStateString);
}
