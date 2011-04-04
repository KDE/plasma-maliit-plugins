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

    //! \brief Returns the secondary label.
    virtual const QString secondaryLabel() const = 0;

    //! \brief Returns the smallest rectangle that contains the button.
    virtual const QRectF &buttonRect() const = 0;

    //! \brief Returns the bounding rectangle for the button.
    virtual const QRectF &buttonBoundingRect() const = 0;

    //! \brief Sets shift and accent. Affects label and/or icon.
    virtual void setModifiers(bool shift,
                              QChar accent = QChar()) = 0;

    //! \brief Sets the button's state to pressed. Selectable has this too.
    virtual void setDownState(bool down) = 0;

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

    //! \brief Activate gravity. Will be reset on next key release.
    virtual void activateGravity() = 0;

    //! \brief Returns whether gravity is active.
    virtual bool isGravityActive() const = 0;

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
