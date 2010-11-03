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

//! Represents a key model with the key's current binding state, and also contains its visible area.
class MImAbstractKey
{
protected:
    static QList<MImAbstractKey *> activeKeys;

public:
    enum ButtonState {
        Normal,  //! Normal is the "up" state for not selected buttons.
        Pressed, //! Button is Pressed when it's set down.
        Selected //! Selected is the "up" state for selected buttons.
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
    virtual void setModifiers(bool shift, QChar accent = QChar()) = 0;

    //! \brief Sets the button's state to pressed. Selectable has this too.
    virtual void setDownState(bool down) = 0;

    //! \brief Selected button
    virtual void setSelected(bool selected) = 0;

    //! \brief Returns the pressed state of the button.
    virtual ButtonState state() const = 0;

    //! \return the key this button represents
    virtual const MImKeyModel &key() const = 0;

    //! \brief Get current active key binding.
    virtual const MImKeyBinding &binding() const = 0;

    //! \brief Tells whether this key represents a dead key.
    virtual bool isDeadKey() const = 0;

    //! \brief Tells whether this key represents a shift key.
    virtual bool isShiftKey() const = 0;

    //! \brief Tells wether this key represends a normal key
    //!        (e.g., q, w, e, r, t, y).
    virtual bool isNormalKey() const = 0;


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

    //! \brief Returns most recent key that became active, and wasn't released yet.
    //!        If no key is active, returns 0.
    static MImAbstractKey* lastActiveKey();

    //! \brief Resets active keys to normal state.
    static void resetActiveKeys();

    //! \brief Filter active keys, by predicate callback.
    //! \returns keys for which the predicate returns true.
    static QList<const MImAbstractKey *> filterActiveKeys(bool (predicate)(const MImAbstractKey *));
};

#endif
