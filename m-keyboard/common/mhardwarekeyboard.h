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



#ifndef MHARDWAREKEYBOARD_H
#define MHARDWAREKEYBOARD_H

#include <Qt>
#include <QObject>
#include <QString>
#include <QEvent>
#include "mxkb.h"
#include "hwkbcharloopsmanager.h"
#include "mkeyboardcommon.h"
#include <MNamespace>

class MInputContextConnection;

/*!
  \brief MHardwareKeyboard implements the hardware keyboard for inputmethod plugin.

  Class MHardwareKeyboard provides enhanced functionality for hardware keyboard, such as
  latch/unlatch and lock/unlock Shift/Fn modifiers; Symbol key functionality; auto-lock Fn
  key when focus is in number/phone number content type etc.  See
  meego-im-framework/doc/src/internals.dox section KeyEventFiltering for more detail.
*/
class MHardwareKeyboard : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor for creating an MHardwareKeyboard object.
     * \param icConnection input context connection to use for sending events and
     * pre-edit and commit strings
     * \param parent Parent object.
     */
    MHardwareKeyboard(MInputContextConnection& icConnection, QObject *parent = 0);

    //! Destructor
    ~MHardwareKeyboard();

    //! \brief Set keyboard type according text entry type.
    void setKeyboardType(M::TextContentType type);

    //! \return current state for \a modifier key in hardware keyboard.
    ModifierState modifierState(Qt::KeyboardModifier modifier) const;

    //! Set auto capitalization state.
    void setAutoCapitalization(bool state);

    /*! \brief Reset internal state of the hardware keyboard code.
     * \post modifiers in clear state
     */
    void reset();

    /*!
     * \brief Filter input key events that come from the hardware keyboard.
     *
     * Exception: the key event may also come from the symbol view.  In that case
     * nativeModifiers is allowed not to be correct (always 0).  nativeScanCode is
     * 0 as well.
     *
     * See meego-im-framework/doc/src/internals.dox section KeyEventFiltering for more detail.
     *
     * \return true if the event was handled, false otherwise
     */
    bool filterKeyEvent(QEvent::Type eventType, Qt::Key keyCode,
                        Qt::KeyboardModifiers modifiers, const QString &text,
                        bool autoRepeat, int count, quint32 nativeScanCode,
                        quint32 nativeModifiers);

    //! \return whether the symbol view is available for the current layout.
    bool symViewAvailable() const;

signals:

    //! \bried Emitted when symbol key is clicked (pressed and released consecutively).
    void symbolKeyClicked();

    //! \brief Emitted when shift state is changed.
    void shiftStateChanged() const;

    /*! \brief Emitted when the state of \a modifier is changed to \a state.
     *
     * Can be emitted also when the modifier state has not changed.
     */
    void modifierStateChanged(Qt::KeyboardModifier modifier, ModifierState state) const;

private:
    /*! \brief Process key release event when symbol modifier is pressed.
     *
     * \return true if the event was processed/consumed, false otherwise
     */
    bool handleReleaseWithSymModifier(Qt::Key keyCode, const QString &text);

    //! \return true if the key press event is such that it should be passed to the application
    bool passKeyOnPress(Qt::Key keyCode, const QString &text) const;

    /*! When X11 modifier bits indicated by \a affect mask in \a value are changed
     * compared to their state in \a previousModifiers, emit modifierStateChanged signal
     * with a modifier corresponding to the changed bit and state that is either
     * ModifierClearState (modifier off) or \a onState (modifier on).  \a shiftMask can be
     * either ShiftMask or LockMask, corresponding to different shift modifier masks used
     * for latching and locking, respectively.
     *
     * Only Fn and Shift modifiers are checked.
     *
     * Also emit shiftStateChanged if shift modifier state changes.
     *
     * \sa modifierStateChanged
     * \sa shiftLevelChanged
     */
    void notifyModifierChange(unsigned char previousModifiers, ModifierState onState,
                              unsigned int shiftMask, unsigned int affect, unsigned int value) const;

    /*! \brief Set latch state of modifiers indicated by \a affect to that indicated by \a value.
     *
     * \param affect X modifier mask
     * \param value modifier states indicated with bits corresponding to \a affect mask
     *
     * \post X11 latch state updated using Xkb.
     * \post changes notified using notifyModifierChange
     * \post autoCaps cleared if needed
     * \post currentLatchedMods updated
     */
    void latchModifiers(unsigned int affect, unsigned int value);

    //! Just like \a latchModifiers but set lock state instead.
    void lockModifiers(unsigned int affect, unsigned int value);

    /*! \brief Cycle Shift/Fn clear/latched/locked states as specified for modifier click.
     *
     * \param keyCode key code of released modifier
     * \param lockMask X11 modifier mask indicating what to lock in latched->locked transition
     * and what to unlock in locked->clear transition.
     * \param latchMask just like lockMask but for clear->latched, latched->locked and
     * latched->clear transitions
     * \param unlockMask mask that indicates what to unlock in clear->latched transition.
     * \param unlatchMask Just like unlockMask but indicates what to unlatch.
     */
    void cycleModifierState(Qt::Key keyCode, unsigned int lockMask,
                            unsigned int latchMask, unsigned int unlockMask,
                            unsigned int unlatchMask);

    /*! \brief Handle Fn/Shift release, cycling state using cycleModifierState if applicable.
     *
     * Parameters are as described for cycleModifierState.
     */
    void handleCyclableModifierRelease(Qt::Key keyCode, unsigned int lockMask,
                                       unsigned int latchMask, unsigned int unlockMask,
                                       unsigned int unlatchMask);

    M::TextContentType keyboardType;
    MXkb mXkb;
    bool autoCaps;
    HwKbCharLoopsManager hwkbCharLoopsManager;
    MInputContextConnection& inputContextConnection;

    //! An attribute of the last key event passed to filterKeyEvent.
    QEvent::Type lastEventType;
    //! An attribute of the last key event passed to filterKeyEvent.
    Qt::Key lastKeyCode;

    // TODO: this isn't used at the moment but might serve as a starting point
    // for the key filtering feature later
    typedef QHash<quint32, bool> PressedKeyMap; // key is native scan code / X keycode
    PressedKeyMap pressedKeys;

    //! X modifier mask of currently latched modifiers.  This approximates X modifier state and
    //! is updated in reset and latchModifiers.
    unsigned char currentLatchedMods;
    //! X modifier mask of currently locked modifiers.  This approximates X modifier state and
    //! is updated from X state on entry to filterKeyEvent and by lockModifiers.
    unsigned char currentLockedMods;

    //! When Sym+<character>... is in progress, index to the character loop currently
    //! being cycled, if any.  -1 otherwise.
    int characterLoopIndex;
    //! Text attribute of the last release event with sym modifier and for which a
    //! character loop was found.
    QString lastSymText;

    bool stateTransitionsDisabled;
    //! Number of shift keys in pressed state at the same time.
    unsigned char shiftsPressed;
    //! If true, user has activated caps lock by pressing two shift keys at once.  Until
    //! both of them have been released, no state transitions are done on shift release.
    //! TODO: what about on Fn release?
    bool shiftShiftCapsLock;

    friend class Ut_MHardwareKeyboard;
    friend class Ft_MHardwareKeyboard;
};

#endif
