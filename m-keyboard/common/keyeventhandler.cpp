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


#include "keyeventhandler.h"
#include "keyevent.h"
#include "mimabstractkey.h"
#include "mimabstractkeyarea.h"

#include <QDebug>

KeyEventHandler::KeyEventHandler(QObject *parent)
    : QObject(parent),
      shiftHeldDown(false),
      ignoreShiftClick(false)
{
}

void KeyEventHandler::addEventSource(MImAbstractKeyArea *eventSource)
{
    bool ok = false;

    ok = connect(eventSource, SIGNAL(keyPressed(const MImAbstractKey *, const KeyContext &)),
                 this, SLOT(handleKeyPress(const MImAbstractKey *, const KeyContext &)));
    Q_ASSERT(ok);

    ok = connect(eventSource, SIGNAL(keyReleased(const MImAbstractKey *, const KeyContext &)),
                 this, SLOT(handleKeyRelease(const MImAbstractKey *, const KeyContext &)));
    Q_ASSERT(ok);

    ok = connect(eventSource, SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)),
                 this, SLOT(handleKeyClick(const MImAbstractKey *, const KeyContext &)));
    Q_ASSERT(ok);

    ok = connect(eventSource, SIGNAL(longKeyPressed(const MImAbstractKey *, const KeyContext &)),
                 this, SLOT(handleLongKeyPress(const MImAbstractKey *, const KeyContext &)));
    Q_ASSERT(ok);
}

void KeyEventHandler::handleKeyPress(const MImAbstractKey *key,
                                     const KeyContext &context)
{
    const KeyEvent event = keyToKeyEvent(*key, QEvent::KeyPress, context);
    emit keyPressed(event);

    if (event.qtKey() == Qt::Key_Shift) {
        shiftHeldDown = true;
        emit shiftPressed(true);
    } else if (shiftHeldDown) {
        ignoreShiftClick = true;
    }
}

void KeyEventHandler::handleKeyRelease(const MImAbstractKey *key,
                                       const KeyContext &context)
{
    const KeyEvent event = keyToKeyEvent(*key, QEvent::KeyRelease, context);

    emit keyReleased(event);

    if (event.qtKey() == Qt::Key_Shift && shiftHeldDown) {
        shiftHeldDown = false;
        emit shiftPressed(false);
    }
}

void KeyEventHandler::handleKeyClick(const MImAbstractKey *key,
                                     const KeyContext &context)
{
    const KeyEvent event = keyToKeyEvent(*key, QEvent::KeyRelease, context);

    if (event.qtKey() == Qt::Key_Shift && ignoreShiftClick) {
        ignoreShiftClick = false; // ignore this event
    } else {
        emit keyClicked(event);
    }
}

void KeyEventHandler::handleLongKeyPress(const MImAbstractKey *key, const KeyContext &context)
{
    const KeyEvent event = keyToKeyEvent(*key, QEvent::KeyPress, context);

    emit longKeyPressed(event);
}

KeyEvent KeyEventHandler::keyToKeyEvent(const MImAbstractKey &key,
                                        QKeyEvent::Type eventType,
                                        const KeyContext &context) const
{
    KeyEvent event;

    // Send always upper case letter if shift held down.
    bool upperCase = context.upperCase || shiftHeldDown;

    if (key.isComposeKey()) {
        event = key.model().toKeyEvent(eventType,
                                       upperCase,
                                       key.isComposing());
    } else if (context.accent.isEmpty()) {
        event = key.model().toKeyEvent(eventType,
                                       upperCase);
    } else {
        event = key.model().toKeyEvent(eventType,
                                       context.accent.at(0),
                                       upperCase);
    }
    event.setCorrectionPosition(context.errorCorrectionPos);

    return event;
}

