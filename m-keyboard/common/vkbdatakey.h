/* * This file is part of m-keyboard *
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



#ifndef VKBDATAKEY_H
#define VKBDATAKEY_H

#include <QString>
#include "keyevent.h"

/*!
 * \brief KeyBinding is a primitive action and label bound to a key
 */
class KeyBinding
{
public:
    //! Key action
    enum KeyAction {
        ActionInsert,
        ActionShift,
        ActionSpace,
        ActionBackspace,
        ActionCycle,
        ActionLayoutMenu,
        ActionSym,
        ActionReturn,
        ActionDecimalSeparator,
        ActionPlusMinusToggle,
        NumActions
    };

    KeyBinding();

    /*!
     * Convert into a KeyEvent
     * \param eventType must be QEvent::KeyPress or QEvent::KeyRelease
     * \param modifiers currently active modifiers
     */
    KeyEvent toKeyEvent(QKeyEvent::Type eventType, Qt::KeyboardModifiers modifiers = Qt::NoModifier) const;

    /*!
     * Convert into a KeyEvent
     * \param eventType must be QEvent::KeyPress or QEvent::KeyRelease
     * \param accent currently active accent character
     * \param modifiers currently active modifiers
     */
    KeyEvent toKeyEvent(QKeyEvent::Type eventType, QChar accent, Qt::KeyboardModifiers modifiers = Qt::NoModifier) const;

    //! \return label in normal case (not accented)
    QString label() const;

    //! \return Secondary label for the binding. Used in phone number keyboard, for example.
    QString secondaryLabel() const;

    //! \return label with a given \a accent
    QString accented(QChar accent) const;

    //! \return true if the key is a dead key, false otherwise
    bool isDead() const;

    //! \return what this key should do
    KeyAction action() const;

private:
    //! Helper method for toKeyEvent methods
    KeyEvent toKeyEventImpl(QKeyEvent::Type eventType,
                            Qt::KeyboardModifiers modifiers,
                            const QString &labelText) const;

    //! What will happen when the key is pressed?
    KeyAction keyAction;
    /*! If specified, cycleset contains a set of characters through which
     * the key cycles when it is pressed consecutively, instead of inserting
     * the label
     */
    QString cycleSet;
    //! Accents enabled for this key
    QString accents;
    //! Labels corresponding to accents
    QString accentedLabels;
    //! Secondary label
    QString secondary_label;
    //! Default key label
    QString keyLabel;
    //! True if key is a dead key
    bool dead;

    friend class KeyboardData;
    friend class Ut_VKBDataKey;
    friend class Ut_KeyButton;
    friend class Ut_MVirtualKeyboard;
};


inline QString KeyBinding::label() const
{
    // TODO: get localized decimal separator from some singleton class
    // which has up-to-date localization setting information
    return keyAction == ActionDecimalSeparator ? "." : keyLabel;
}

inline QString KeyBinding::secondaryLabel() const
{
    return secondary_label;
}

inline  bool KeyBinding::isDead() const
{
    return dead;
}

inline KeyBinding::KeyAction KeyBinding::action() const
{
    return keyAction;
}


/*!
 * \brief VKBDataKey is a container for bindings of a key in a keyboard layout
 *
 * Bindings are differentiated with attributes such as shift and alt
 */
class VKBDataKey
{
public:
    VKBDataKey();
    ~VKBDataKey();

    //! \return binding based on given attributes
    const KeyBinding *binding(bool shift = false) const;

    /*!
     * Convert into a KeyEvent
     * \param eventType must be QEvent::KeyPress or QEvent::KeyRelease
     * \param shift whether shift modifier is on
     */
    KeyEvent toKeyEvent(QKeyEvent::Type eventType, bool shift = false) const;

    /*!
     * Convert into a KeyEvent
     * \param eventType must be QEvent::KeyPress or QEvent::KeyRelease
     * \param accent currently active accent character
     * \param shift whether shift modifier is on
     */
    KeyEvent toKeyEvent(QKeyEvent::Type eventType, QChar accent, bool shift = false) const;

private:
    enum  {
        NoShift = 0,
        Shift   = 1,
        NumBindings
    };

    // All indices always contain a binding object, though more than one index may contain
    // the same binding
    const KeyBinding *bindings[NumBindings];
    friend class KeyboardData;
    friend class Ut_VKBDataKey;
    friend class Ut_KeyButton;
};


inline const KeyBinding *VKBDataKey::binding(bool shift) const
{
    return bindings[shift ? Shift : NoShift];
}


#endif
