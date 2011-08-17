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



#ifndef MIMKEYMODEL_H
#define MIMKEYMODEL_H

#include "keyevent.h"

#include <QSize>
#include <QString>

/*!
 * \brief MImKeyBinding is a primitive action and label bound to a key
 */
class MImKeyBinding
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
        ActionTab,
        ActionCommit,
        ActionSwitch,
        ActionOnOffToggle,
        ActionCompose,
        NumActions
    };

    MImKeyBinding();

    //! \brief Create simple insert-action binding with a given label
    explicit MImKeyBinding(const QString &label);

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

    /*!
     * Convert into a KeyEvent
     * \param eventType must be QEvent::KeyPress or QEvent::KeyRelease
     * \param isComposing currently active state is composing
     * \param modifiers currently active modifiers
     */
    KeyEvent toKeyEvent(QKeyEvent::Type eventType, bool isComposing, Qt::KeyboardModifiers modifiers = Qt::NoModifier) const;

    //! \return label in normal case (not accented)
    QString label() const;

    //! \return Secondary label for the binding. Used in phone number keyboard, for example.
    QString secondaryLabel() const;

    //! \return label with a given \a accent
    QString accented(QChar accent) const;

    //! \return accented labels available for the key binding
    QString accentedLabels() const;

    //! \return true if the key is a dead key, false otherwise
    bool isDead() const;

    //! \return what this key should do
    KeyAction action() const;

    //! \return extra labels available for the key binding
    QString extendedLabels() const;

    //! \return true if the key is a quick pick key, i.e. it should automatically
    //! close the view it was "picked" from. Only symbol view currently supports this.
    bool isQuickPick() const;

    //! \return true if the key needs right-to-left representation during rendering
    bool isRtl() const;

private:
    //! Helper method for toKeyEvent methods
    KeyEvent toKeyEventImpl(QKeyEvent::Type eventType,
                            Qt::KeyboardModifiers modifiers,
                            const QString &labelText,
                            bool isComposing = false) const;

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
    QString accented_labels;
    //! Extra labels of the extended_labels attribute
    QString extended_labels;
    //! Secondary label
    QString secondary_label;
    //! Default key label
    QString keyLabel;
    //! True if key is a dead key
    bool dead;
    //! True if this is a quickpick key for symbol view page.
    bool quickPick;
    //! True if this is a compose key.
    bool compose;
    //! True if this is an rtl key.
    bool rtl;

    friend class KeyboardData;
    friend class Ut_MImKeyModel;
    friend class Ut_MImKey;
    friend class Ut_MVirtualKeyboard;
};


inline QString MImKeyBinding::label() const
{
    // TODO: get localized decimal separator from some singleton class
    // which has up-to-date localization setting information
    return keyAction == ActionDecimalSeparator ? "." : keyLabel;
}

inline QString MImKeyBinding::secondaryLabel() const
{
    return secondary_label;
}

inline bool MImKeyBinding::isDead() const
{
    return dead;
}

inline bool MImKeyBinding::isQuickPick() const
{
    return quickPick;
}

inline MImKeyBinding::KeyAction MImKeyBinding::action() const
{
    return keyAction;
}

inline QString MImKeyBinding::extendedLabels() const
{
    return extended_labels;
}

inline bool MImKeyBinding::isRtl() const
{
    return rtl;
}


/*!
 * \brief MImKeyModel is a container for bindings of a key in a keyboard layout
 *
 * Bindings are differentiated with attributes such as shift and alt
 */
class MImKeyModel
{
public:
    //! Style Type
    enum StyleType {
        NormalStyle,   //!< Key uses normal style (character key)
        SpecialStyle,  //!< Key uses special style (return, shift, etc.)
        DeadkeyStyle   //!< Key uses deadkey style
    };

    //! Width type
    enum WidthType {
        Small,         //!< Key uses small width
        Medium,        //!< Key uses medium width
        Large,         //!< Key uses large width
        XLarge,        //!< Key uses extra large width
        XxLarge,       //!< Key uses extra-extra large keys
        Stretched      //!< Key consumes remaining width
    };

    /*
     * \brief Constructs new object
     * \param type The style type for button.
     * \param widthType The width type for the button.
     * \param isFixed Contains true if button should use fixed width type.
     * \param isRtl Contains true if button should use RTL icon.
     */
    explicit MImKeyModel(StyleType type = NormalStyle, WidthType widthType = Medium,
                        bool isFixed = false, bool isRtl = false,
                        const QString &id = QString());

    ~MImKeyModel();

    //! \brief Set \a binding as the binding for given \a shift attribute
    void setBinding(const MImKeyBinding &binding, bool shift);

    //! \return binding based on given attributes
    const MImKeyBinding *binding(bool shift = false) const;

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

    /*!
     * Convert into a KeyEvent
     * \param eventType must be QEvent::KeyPress or QEvent::KeyRelease
     * \param isComposing currently active state is composing
     * \param shift whether shift modifier is on
     */
    KeyEvent toKeyEvent(QKeyEvent::Type eventType, bool shift, bool isComposing) const;

    //! Returns the style type.
    StyleType style() const;

    //! Returns the width type.
    WidthType width() const;

    //! Returns true if button uses fixed width type.
    bool isFixedWidth() const;

    //! Returns true if button uses RTL icon.
    bool rtl() const;

    //! Returns key's identifier
    QString id() const;

    //! Set a temporary override for the binding
    void overrideBinding(const MImKeyBinding *binding, bool shift);

private:
    enum  {
        NoShift = 0,
        Shift   = 1,
        NumBindings
    };

    // All indices always contain a binding object, though more than one index may contain
    // the same binding
    const MImKeyBinding *bindings[NumBindings];

    // Active bindings (changeable by overrideBinding or setBinding):
    // NOTE: this are only aliases, they are not owned. No need to
    // delete them!
    const MImKeyBinding *activeBindings[NumBindings];

    StyleType mStyle;

    WidthType mWidthType;

    //! Contains true if button uses fixed width.
    bool isFixed;

    //! Contains true if button uses RTL icon.
    bool isRtl;

    //! Contains key's identifier
    QString keyId;

};


inline const MImKeyBinding *MImKeyModel::binding(bool shift) const
{
    return activeBindings[shift ? Shift : NoShift];
}

#endif
