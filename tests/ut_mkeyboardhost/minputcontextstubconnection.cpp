/* * This file is part of meego-keyboard *
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

#include "minputcontextstubconnection.h"
#include <mnamespace.h>
#include <QKeyEvent>

MInputContextStubConnection::MInputContextStubConnection()
{
    clear();
    predictionEnabled_ = true;
    correctionEnabled_ = true;
    autoCapitalizationEnabled_ = true;
    contentType_ = 0;
    inputmethodMode_ = M::InputMethodModeNormal;
    keyRedirectionEnabled = false;
}

MInputContextStubConnection::~MInputContextStubConnection()
{
    qDeleteAll(keyEvents);
    keyEvents.clear();
}

void MInputContextStubConnection::clear()
{
    preedit.clear();
    commit.clear();
    qDeleteAll(keyEvents);
    keyEvents.clear();

    surroundingString.clear();
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

void MInputContextStubConnection::sendPreeditString(const QString &string, PreeditFace preeditFace)
{
    Q_UNUSED(preeditFace);

    preedit = string;
    ++sendPreeditCalls;
}

void MInputContextStubConnection::sendCommitString(const QString &string)
{
    commit += string;
    ++sendCommitStringCalls;
}


void MInputContextStubConnection::sendKeyEvent(const QKeyEvent &keyEvent_)
{
    keyEvents.append(new QKeyEvent(keyEvent_));
    ++sendKeyEventCalls;
}

void MInputContextStubConnection::notifyImInitiatedHiding()
{
    ++notifyImInitiatedHidingCalls;
}

int MInputContextStubConnection::contentType(bool &val)
{
    val = true;
    return contentType_;
}

bool MInputContextStubConnection::correctionEnabled(bool &val)
{
    val = true;
    return correctionEnabled_;
}

bool MInputContextStubConnection::predictionEnabled(bool &val)
{
    val = true;
    return predictionEnabled_;
}

bool MInputContextStubConnection::surroundingText(QString &text, int &cursorPosition)
{
    text = surroundingString;
    cursorPosition = cursorPos;
    return true;
}

int MInputContextStubConnection::inputMethodMode(bool &valid)
{
    valid = true;
    return inputmethodMode_;
}

bool MInputContextStubConnection::hasSelection(bool &valid)
{
    valid = true;
    return textSelected;
}

QRect MInputContextStubConnection::preeditRectangle(bool &valid)
{
    valid = true;
    return QRect();
}

bool MInputContextStubConnection::autoCapitalizationEnabled(bool &val)
{
    val = true;
    return autoCapitalizationEnabled_;
}

void MInputContextStubConnection::copy()
{
    ++copyCalls;
}

void MInputContextStubConnection::paste()
{
    ++pasteCalls;
}

void MInputContextStubConnection::setGlobalCorrectionEnabled(bool enabled)
{
    globalCorrectionEnabled = enabled;
    ++setGlobalCorrectionEnabledCalls;
}

void MInputContextStubConnection::setRedirectKeys(bool enabled)
{
    keyRedirectionEnabled = enabled;
}

void MInputContextStubConnection::setDetectableAutoRepeat(bool /*enabled*/)
{
}
