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



#ifndef MIMABSTRACTKEY_H
#define MIMABSTRACTKEY_H

#include "mimkeymodel.h"
#include <QList>
#include <QSharedPointer>
#include <mkeyoverride.h>

class QRect;
class QPainter;
class MImAbstractKey;
class MScalableImage;

//! Visitor interface that can be used for MImAbstractKey::visitActiveKeys
class MImAbstractKeyVisitor
{
public:
    virtual bool operator()(MImAbstractKey *key) = 0;
};

//! Represents a key model with the key's current binding state, and also contains its visible area.
class MImAbstractKey
{
protected:
    static QList<MImAbstractKey *> activeKeys;

public:
    enum ButtonState {
        Normal,  //! Normal is the "up" state for not selected buttons.
        Pressed, //! Button is Pressed when it's set down.
        Selected, //! Selected is the "up" state for selected buttons.
        Disabled, //! Button does not interact with user.
    };

    virtual ~MImAbstractKey();

    //! \brief Returns current label. It is affected by active modifiers.
    virtual const QString label() const = 0;
    
    //! \brief Returns rendering version of current label. It is logically the same with label(), but shown with modifier or with different glyph
    virtual const QString renderingLabel() const = 0;

    //! \brief Returns the secondary label.
    virtual const QString secondaryLabel() const = 0;

    //! \brief Returns the smallest rectangle that contains the button.
    virtual const QRectF &buttonRect() const = 0;

    //! \brief Returns the bounding rectangle for the button.
    virtual const QRectF &buttonBoundingRect() const = 0;

    //! \brief Sets shift and accent. Affects label and/or icon.
    virtual void setModifiers(bool shift,
                              QChar accent = QChar()) = 0;

    //! \brief Gets current shift and accent state.
    virtual bool modifiers(QChar *accent = 0) const = 0;

    //! \brief Selected button
    virtual void setSelected(bool selected) = 0;

    //! \brief Sets button to be composing.
    //! \note Only for \a isComposeKey() key.
    virtual void setComposing(bool composing) = 0;

    //! \brief Returns the pressed state of the button.
    virtual ButtonState state() const = 0;

    //! \brief Returns whether the key is during composing state.
    //! \note Only for \a isComposeKey() key.
    virtual bool isComposing() const = 0;

    //! \return the key this button represents
    virtual const MImKeyModel &model() const = 0;

    //! \brief Get current active key binding.
    virtual const MImKeyBinding &binding() const = 0;

    //! \brief Tells whether this key represents a dead key.
    virtual bool isDeadKey() const = 0;

    //! \brief Tells whether this key represents a shift key.
    virtual bool isShiftKey() const = 0;

    //! \brief Tells wether this key represends a normal key
    //!        (e.g., q, w, e, r, t, y).
    virtual bool isNormalKey() const = 0;

    //! \brief Tells whether emitter view should close after clicking this key.
    virtual bool isQuickPick() const = 0;

    //! \brief Tells whether the key is a compose key.
    virtual bool isComposeKey() const = 0;

    //! \brief Tells whether the key is a backspace key.
    virtual bool isBackspaceKey() const = 0;

    //! \brief Tells whether the key is auto repeat key.
    virtual bool isAutoRepeatKey() const = 0;

    //! \brief Called when a new touchpoint was registered on this button.
    //! \returns true if the counter could be increased.
    //!          Cannot exceed total active touchpoint limit
    virtual bool increaseTouchPointCount() = 0;

    //! \brief Called when a touchpoint left the button.
    //! \returns true if the counter could be decreased.
    //!          Cannot become negative.
    virtual bool decreaseTouchPointCount() = 0;

    //! \brief Resets touchpoint count.
    virtual void resetTouchPointCount() = 0;

    //! \brief Get current touchpoint count.
    virtual int touchPointCount() const = 0;

    //! \brief Return background image according to current mode and style.
    virtual const MScalableImage *backgroundImage() const = 0;

    //! \brief Return background image corresponding to normal mode and current style.
    //! Note: this method also ignores key overrides.
    virtual const MScalableImage *normalBackgroundImage() const = 0;

    //! \brief Set custom key override in order to change visual appearance, e.g. label, icon etc.
    virtual void setKeyOverride(const QSharedPointer<MKeyOverride> &override) = 0;

    //! \brief Return the attached custom key override.
    virtual QSharedPointer<MKeyOverride> keyOverride() const = 0;

    //! \brief Reset any custom key override.
    virtual void resetKeyOverride() = 0;

    //! \brief Return true if key should reach on touch events.
    //! \sa setKeyOverride
    //! \sa MKeyOverride
    virtual bool enabled() const;

    //! \brief update custom key override attributes.
    virtual void updateOverrideAttributes(MKeyOverride::KeyOverrideAttributes changedAttributes) = 0;

    //! \brief Returns most recent key that became active, and wasn't released yet.
    //!        If no key is active, returns 0.
    static MImAbstractKey* lastActiveKey();

    //! \brief Resets active keys to normal state.
    //! \warning Be careful when using this. Some key areas may have changed
    //!          their visual appearance according to key modifiers. However,
    //!          if all active keys are reset, those key areas might not be
    //!          aware of this change.
    //!          It is usually better to write a specialized visitor, unless
    //!          you know what you're doing.
    //! \sa visitActiveKeys, MImAbstractKeyVisitor
    static void resetActiveKeys();

    //! \brief Visit active keys.
    //! \param visitor must overload 'bool operator()(MImAbstractKey *key)',
    //!        aborts to visit next key when true is returned.
    static void visitActiveKeys(MImAbstractKeyVisitor *visitor);
};

#endif
