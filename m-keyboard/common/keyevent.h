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



#ifndef VKB_KEYEVENT_H
#define VKB_KEYEVENT_H

#include <QKeyEvent>
#include <QPoint>
#include <Qt>

//! Internal keyboard event class, basically a superset of QKeyEvent
class KeyEvent
{
public:
    //! Special keys defined by virtual keyboard and not covered by Qt::Key
    enum SpecialKey {
        NotSpecial,
        Copy,
        Paste,
        LayoutMenu,
        CycleSet,
        Sym,
        Commit,
        Switch,
        OnOffToggle,
        ChangeSign,
        NumSpecialKeys
    };

    //! Constructor that takes all attributes
    KeyEvent(const QString &text = QString(),
             QKeyEvent::Type type = QEvent::KeyRelease,
             Qt::Key qtKey = Qt::Key_unknown,
             SpecialKey specialKey = NotSpecial,
             Qt::KeyboardModifiers modifiers = Qt::NoModifier,
             const QPoint &correctionPos = QPoint());

    //! Constructor that copies another event, except for type
    KeyEvent(const KeyEvent &other, QKeyEvent::Type type);

    /*!
     * \return Qt key code. We always use Qt::Key_unknown in case key code is
     * not used, unlike QKeyEvent::key().  Also, this is not used for normal
     * alphanumeric keys; for those the text is all there is.
     */
    Qt::Key qtKey() const;

    //! \return special key code, which can augment or override Qt key code
    SpecialKey specialKey() const;

    //! \return modifiers
    Qt::KeyboardModifiers modifiers() const;

    /*!
     * \return Unicode text the key generated. Can be empty, in which case
     * specialKey() or qtKey() should return something useful.
     */
    QString text() const;

    /*!
     * \return this event converted into a QKeyEvent.  Results are undefined
     * if specialKey() returns other than NotSpecial.
     */
    QKeyEvent toQKeyEvent() const;

    //! \return event type, either QEvent::KeyPress or QEvent::KeyRelease
    QKeyEvent::Type type() const;

    bool operator==(const KeyEvent &other) const;

    /*!
     * \brief Sets the event's layout position that is delivered to error correction engine.
     *
     * The position is relative to the emitting widget.
     * \sa correctionPosition
     */
    void setCorrectionPosition(const QPoint &point);

    /*!
     * \brief Returns event's layout position given to error correction engine.
     *
     * The position is relative to the emitting widget.
     * \sa setCorrectionPosition
     */
    QPoint correctionPosition() const;

protected:
    QKeyEvent::Type m_type;
    Qt::Key m_qtKey;
    SpecialKey m_specialKey;
    QString m_text;
    Qt::KeyboardModifiers m_modifiers;
    QPoint m_correctionPos;
};

#endif
