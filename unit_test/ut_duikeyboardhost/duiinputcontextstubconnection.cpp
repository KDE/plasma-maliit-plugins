/* * This file is part of dui-keyboard *
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

#include "duiinputcontextstubconnection.h"
#include <duinamespace.h>
#include <QKeyEvent>

DuiInputContextStubConnection::DuiInputContextStubConnection()
{
    clear();
    predictionEnabled_ = true;
    correctionEnabled_ = true;
    autoCapitalizationEnabled_ = true;
    contentType_ = 0;
    inputmethodMode_ = Dui::InputMethodModeNormal;
    keyRedirectionEnabled = false;
}

DuiInputContextStubConnection::~DuiInputContextStubConnection()
{
    qDeleteAll(keyEvents);
    keyEvents.clear();
}

void DuiInputContextStubConnection::clear()
{
    preedit.clear();
    commit.clear();
    qDeleteAll(keyEvents);
    keyEvents.clear();

    surrodingString.clear();
    cursorPos = 0;

    sendPreeditCalls = 0;
    sendCommitStringCalls = 0;
    sendKeyEventCalls = 0;
    notifyImInitiatedHidingCalls = 0;
    setGlobalCorrectionEnabledCalls = 0;
    predictionEnabledCalls = 0;
    correctionEnabledCalls = 0;
    contentTypeCalls = 0;
    copyCalls = 0;
    pasteCalls = 0;
    textSelected = false;
    keyRedirectionEnabled = false;
}

void DuiInputContextStubConnection::sendPreeditString(const QString &string, PreeditFace preeditFace)
{
    Q_UNUSED(preeditFace);

    preedit = string;
    ++sendPreeditCalls;
}

void DuiInputContextStubConnection::sendCommitString(const QString &string)
{
    commit += string;
    ++sendCommitStringCalls;
}


void DuiInputContextStubConnection::sendKeyEvent(const QKeyEvent &keyEvent_)
{
    keyEvents.append(new QKeyEvent(keyEvent_));
    ++sendKeyEventCalls;
}

void DuiInputContextStubConnection::notifyImInitiatedHiding()
{
    ++notifyImInitiatedHidingCalls;
}

int DuiInputContextStubConnection::contentType(bool &val)
{
    val = true;
    return contentType_;
}

bool DuiInputContextStubConnection::correctionEnabled(bool &val)
{
    val = true;
    return correctionEnabled_;
}

bool DuiInputContextStubConnection::predictionEnabled(bool &val)
{
    val = true;
    return predictionEnabled_;
}

bool DuiInputContextStubConnection::surroundingText(QString &text, int &cursorPosition)
{
    text = surrodingString;
    cursorPosition = cursorPos;
    return true;
}

int DuiInputContextStubConnection::inputMethodMode(bool &valid)
{
    valid = true;
    return inputmethodMode_;
}

bool DuiInputContextStubConnection::hasSelection(bool &valid)
{
    valid = true;
    return textSelected;
}

QRect DuiInputContextStubConnection::preeditRectangle(bool &valid)
{
    valid = true;
    return QRect();
}

bool DuiInputContextStubConnection::autoCapitalizationEnabled(bool &val)
{
    val = true;
    return autoCapitalizationEnabled_;
}

void DuiInputContextStubConnection::copy()
{
    ++copyCalls;
}

void DuiInputContextStubConnection::paste()
{
    ++pasteCalls;
}

void DuiInputContextStubConnection::setGlobalCorrectionEnabled(bool enabled)
{
    globalCorrectionEnabled = enabled;
    ++setGlobalCorrectionEnabledCalls;
}

void DuiInputContextStubConnection::setRedirectKeys(bool enabled)
{
    keyRedirectionEnabled = enabled;
}
