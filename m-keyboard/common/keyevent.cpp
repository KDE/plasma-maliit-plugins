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



#include "keyevent.h"

KeyEvent::KeyEvent(const QString &text, QKeyEvent::Type type, Qt::Key qtKey, SpecialKey specialKey,
                   Qt::KeyboardModifiers modifiers,
                   const QPoint &correctionPos,
                   const QPointF &scenePos)
    : m_type(type),
      m_qtKey(qtKey),
      m_specialKey(specialKey),
      m_text(text),
      m_modifiers(modifiers),
      m_correctionPos(correctionPos),
      m_scenePos(scenePos)
{
}

KeyEvent::KeyEvent(const KeyEvent &other, QKeyEvent::Type type)
    : m_type(type),
      m_qtKey(other.m_qtKey),
      m_specialKey(other.m_specialKey),
      m_text(other.m_text),
      m_modifiers(other.m_modifiers),
      m_correctionPos(other.m_correctionPos),
      m_scenePos(other.m_scenePos)
{
}

Qt::Key KeyEvent::qtKey() const
{
    return m_qtKey;
}

KeyEvent::SpecialKey KeyEvent::specialKey() const
{
    return m_specialKey;
}

Qt::KeyboardModifiers KeyEvent::modifiers() const
{
    return m_modifiers;
}

QKeyEvent KeyEvent::toQKeyEvent() const
{
    return QKeyEvent(m_type, m_qtKey, m_modifiers, m_text, false, qMax(m_text.size(), 1));
}

QKeyEvent::Type KeyEvent::type() const
{
    return m_type;
}

QString KeyEvent::text() const
{
    return m_text;
}

bool KeyEvent::operator==(const KeyEvent &other) const
{
    return (m_qtKey == other.m_qtKey)
           && (m_specialKey == other.m_specialKey)
           && (m_modifiers == other.m_modifiers)
           && (m_type == other.m_type)
           && (m_text == other.m_text);
}

void KeyEvent::setCorrectionPosition(const QPoint &point)
{
    m_correctionPos = point;
}

QPoint KeyEvent::correctionPosition() const
{
    return m_correctionPos;
}

void KeyEvent::setScenePosition(const QPointF &point)
{
    m_scenePos = point;
}

QPointF KeyEvent::scenePosition() const
{
    return m_scenePos;
}
