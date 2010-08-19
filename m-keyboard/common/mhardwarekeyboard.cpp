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

#include <QDebug>
#include <QX11Info>
#include <QTextCodec>
#include <algorithm>

#include <minputcontextconnection.h>

#include "mhardwarekeyboard.h"
#include "layoutsmanager.h"

#include <X11/X.h>
#undef KeyPress
#undef KeyRelease
#include <X11/XKBlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

namespace
{
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    const Qt::Key FnLevelKey = Qt::Key_AltGr;
    const Qt::Key SymKey = Qt::Key_Multi_key;
    const unsigned int SymModifierMask = Mod4Mask;
    const unsigned int FnModifierMask = Mod5Mask;
    const unsigned int longPressTime = 600; // in milliseconds

    const unsigned short RepeatDelay(660);   // in milliseconds
    const unsigned short RepeatInterval(25); // in milliseconds

    const int KeyCodeShift = 3;
    const int KeyCodeMask = 7;

    inline void setKeyBit(XkbDescPtr description, KeyCode keyCode)
    {
        description->ctrls->per_key_repeat[keyCode >> KeyCodeShift] |=
            1 << (keyCode & KeyCodeMask);
    }

    inline void clearKeyBit(XkbDescPtr description, KeyCode keyCode)
    {
        description->ctrls->per_key_repeat[keyCode >> KeyCodeShift] &=
            ~(1 << (keyCode & KeyCodeMask));
    }
};

MHardwareKeyboard::MHardwareKeyboard(MInputContextConnection& icConnection, QObject *parent)
    : QObject(parent),
      currentKeyboardType(M::FreeTextContentType),
      autoCaps(false),
      inputContextConnection(icConnection),
      lastEventType(QEvent::KeyRelease),
      currentLatchedMods(0),
      currentLockedMods(0),
      characterLoopIndex(-1),
      stateTransitionsDisabled(false),
      shiftsPressed(0),
      shiftShiftCapsLock(false),
      longPressTimer(this),
      imMode(M::InputMethodModeNormal),
      fnPressed(false)
{
    longPressTimer.setSingleShot(true);
    longPressTimer.setInterval(longPressTime);
    connect(&longPressTimer, SIGNAL(timeout()), this, SLOT(handleLongPressTimeout()));
}


MHardwareKeyboard::~MHardwareKeyboard()
{
}


void MHardwareKeyboard::setKeyboardType(M::TextContentType type)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << type;
    if (currentKeyboardType == type)
        return;
    currentKeyboardType = type;

    // To support dynamic keyboard type changes we need to set correct state
    // for modifiers according current keyboard type.
    latchModifiers(ShiftMask | FnModifierMask, 0);
    switch (currentKeyboardType) {
    case M::NumberContentType:
    case M::PhoneNumberContentType:
        // With number and phone number content type Fn must be permanently locked
        lockModifiers(FnModifierMask, FnModifierMask);
        stateTransitionsDisabled = true;
        break;
    default:
        stateTransitionsDisabled = false;
        // clear locked modifiers for other keyboard types.
        lockModifiers(LockMask | FnModifierMask, 0);
        break;
    }
}

M::TextContentType MHardwareKeyboard::keyboardType() const
{
    return currentKeyboardType;
}


void MHardwareKeyboard::toggleCustomAutoRepeat(const bool enable)
{
    XkbDescPtr description(XkbAllocKeyboard());
    if (!description) {
        qWarning() << "Unable to allocate Xkb keyboard description for autorepeat setup.";
        return;
    }

    const unsigned int neededControls(XkbRepeatKeysMask | XkbPerKeyRepeatMask);
    if (Success != XkbGetControls(QX11Info::display(), neededControls, description)) {
        qWarning() << "Failure to query Xkb server controls for autorepeat setup.";
        XkbFreeKeyboard(description, 0, True);
        return;
    }

    if (!description->ctrls) {
        qWarning() << "Unable to query Xkb server controls for autorepeat setup.";
        XkbFreeKeyboard(description, 0, True);
        return;
    }

    if (False == XkbChangeEnabledControls(QX11Info::display(), XkbUseCoreKbd,
                                          XkbRepeatKeysMask, XkbRepeatKeysMask)) {
        qWarning() << "Unable to enable controls for autorepeat.";
        XkbFreeKeyboard(description, 0, True);
        return;
    }

    description->ctrls->repeat_delay = RepeatDelay;
    description->ctrls->repeat_interval = RepeatInterval;

    std::fill(description->ctrls->per_key_repeat,
              description->ctrls->per_key_repeat + XkbPerKeyBitArraySize, enable ? 0 : 0xff);

    if (enable) {
        static const KeySym repeatableKeys[] = { XK_BackSpace, XK_Left, XK_Up, XK_Right, XK_Down };

        for (unsigned int i = 0; i < ELEMENTS(repeatableKeys); ++i) {
            const KeyCode repeatableKeyCode(XKeysymToKeycode(QX11Info::display(), repeatableKeys[i]));
            if (repeatableKeyCode) {
                setKeyBit(description, repeatableKeyCode);
            } else {
                qWarning() << "Unable to make keysym" << repeatableKeys[i] << "repeatable: no keycode found.";
            }
        }
    } else {
        static const KeySym nonRepeatableKeys[] = {
            XK_ISO_Level3_Shift, // Fn
            XK_Multi_key, // Sym
            XK_Shift_L,
            XK_Shift_R,
            XK_Control_R
        };

        for (unsigned int i = 0; i < ELEMENTS(nonRepeatableKeys); ++i) {
            const KeyCode nonRepeatableKeyCode(XKeysymToKeycode(QX11Info::display(), nonRepeatableKeys[i]));
            if (nonRepeatableKeyCode) {
                clearKeyBit(description, nonRepeatableKeyCode);
            } else {
                qWarning() << "Unable to make keysym" << nonRepeatableKeys[i] << "non-repeatable: no keycode found.";
            }
        }
    }

    if (False == XkbSetControls(QX11Info::display(), neededControls, description)) {
        qWarning() << "Unable to set Xkb server controls for autorepeat setup.";
    }

    XkbFreeKeyboard(description, 0, True);

    inputContextConnection.setDetectableAutoRepeat(enable);
}

void MHardwareKeyboard::enable()
{
    qDebug() << __PRETTY_FUNCTION__;

    // We don't know how many requirements for direct mode will be yet besides
    // symbol view is required for symbol key. So redirect all the keys to plugin,
    // but now only handle symbol key.
    if (imMode != M::InputMethodModeDirect) {
        toggleCustomAutoRepeat(true);

        shiftShiftCapsLock = false;
        shiftsPressed = 0;
        pressedKeys.clear();
        fnPressed = false;
        autoCaps = false;

        // We reset state without using latch/lockModifiers in order to force notification
        // (to make sure whoever is listening is in sync with us).
        currentLatchedMods = 0;
        mXkb.latchModifiers(ShiftMask | FnModifierMask, 0);
        emit modifierStateChanged(Qt::ShiftModifier, ModifierClearState);
        switch (currentKeyboardType) {
        case M::NumberContentType:
        case M::PhoneNumberContentType:
            // With number and phone number content type Fn must be permanently locked
            currentLockedMods = FnModifierMask;
            mXkb.lockModifiers(FnModifierMask, FnModifierMask);
            emit modifierStateChanged(FnLevelModifier, ModifierLockedState);
            stateTransitionsDisabled = true;
            break;
        default:
            stateTransitionsDisabled = false;
            // clear locked modifiers for other keyboard types.
            currentLockedMods = 0;
            mXkb.lockModifiers(LockMask | FnModifierMask, 0);
            emit modifierStateChanged(FnLevelModifier, ModifierClearState);
            break;
        }
    }

    inputContextConnection.setRedirectKeys(true);
}


void MHardwareKeyboard::disable()
{
    qDebug() << __PRETTY_FUNCTION__;

    inputContextConnection.setRedirectKeys(false);
    // Unlock and unlatch everything.  If for example non-Qt X application gets focus
    // after this focus out, there is no way to unlock Lock modifier using the
    // physical keyboard.  So better clear the state as well as we can.
    lockModifiers(LockMask | FnModifierMask, 0);
    latchModifiers(ShiftMask | FnModifierMask, 0);

    toggleCustomAutoRepeat(false);
}


void MHardwareKeyboard::reset()
{
    qDebug() << __PRETTY_FUNCTION__;
}


bool MHardwareKeyboard::actionOnPress(Qt::Key keyCode) const
{
    static const Qt::Key pressPassKeys[] = {
        Qt::Key_Backspace, Qt::Key_Delete, Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down };
    static const Qt::Key * const keysEnd = pressPassKeys + ELEMENTS(pressPassKeys);

    return keysEnd != std::find(pressPassKeys, keysEnd, keyCode);
}

bool MHardwareKeyboard::passKeyOnPress(Qt::Key keyCode, const QString &text,
                                       unsigned int nativeScanCode) const
{
    const unsigned int shiftLevel(3); // Fn.
    return (text.isEmpty() && keycodeToString(nativeScanCode, shiftLevel).isEmpty())
        || actionOnPress(keyCode);
}


void MHardwareKeyboard::notifyModifierChange(
    unsigned char previousModifiers, ModifierState onState, unsigned int shiftMask,
    unsigned int affect, unsigned int value) const
{
    if ((affect & shiftMask) && ((value & shiftMask) != (previousModifiers & shiftMask))) {
        emit shiftStateChanged();
        emit modifierStateChanged(
            Qt::ShiftModifier, (value & shiftMask) ? onState : ModifierClearState);
    }
    if ((affect & FnModifierMask)
        && ((value & FnModifierMask) != (previousModifiers & FnModifierMask))) {
        emit modifierStateChanged(
            FnLevelModifier, (value & FnModifierMask) ? onState : ModifierClearState);
    }
}


void MHardwareKeyboard::latchModifiers(unsigned int affect, unsigned int value)
{
    mXkb.latchModifiers(affect, value);
    const unsigned int savedLatchedMods = currentLatchedMods;
    currentLatchedMods = (currentLatchedMods & ~affect) | (value & affect);
    if (!(currentLatchedMods & ShiftMask)) {
        autoCaps = false;
    }
    notifyModifierChange(savedLatchedMods, ModifierLatchedState, ShiftMask, affect, value);
}


void MHardwareKeyboard::lockModifiers(unsigned int affect, unsigned int value)
{
    mXkb.lockModifiers(affect, value);
    const unsigned int savedLockedMods = currentLockedMods;
    currentLockedMods = (currentLockedMods & ~affect) | (value & affect);
    notifyModifierChange(savedLockedMods, ModifierLockedState, LockMask, affect, value);
}


void MHardwareKeyboard::cycleModifierState(Qt::Key keyCode, unsigned int lockMask,
                                           unsigned int latchMask, unsigned int unlockMask,
                                           unsigned int unlatchMask)
{
    if (currentLockedMods & lockMask) {
        lockModifiers(lockMask, 0);
    } else if (currentLatchedMods & latchMask) {
        const bool savedAutoCaps = autoCaps;
        latchModifiers(latchMask, 0);
        if (!((keyCode == Qt::Key_Shift) && savedAutoCaps)) {
            lockModifiers(lockMask, lockMask);
        }
    } else {
        lockModifiers(unlockMask, 0);
        latchModifiers(latchMask | unlatchMask, latchMask);
    }
}


void MHardwareKeyboard::handleCyclableModifierRelease(
    Qt::Key keyCode, unsigned int lockMask, unsigned int latchMask,
    unsigned int unlockMask, unsigned int unlatchMask)
{
    if (!stateTransitionsDisabled && (lastKeyCode == keyCode) && (lastEventType == QEvent::KeyPress)) {
        cycleModifierState(keyCode, lockMask, latchMask, unlockMask, unlatchMask);
    }
}

bool MHardwareKeyboard::filterKeyPress(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                                       QString text, bool autoRepeat, int count,
                                       quint32 nativeScanCode, quint32 nativeModifiers)
{
    bool eaten = false;

    // eat Fn/Shift press when Shift/Fn is held
    if (keyCode == Qt::Key_Shift && fnPressed) {
        ++shiftsPressed;
        return true;
    }
    if (keyCode == FnLevelKey && shiftsPressed) {
        fnPressed = true;
        return true;
    }

    // switch keyboard map for Space press when (only) Control is held
    if (keyCode == Qt::Key_Space
        && (modifiers & Qt::ControlModifier)
        && (!fnPressed && !shiftsPressed)) {
        switchKeyMap();
        return true;
    }

    if (imMode == M::InputMethodModeDirect) {
        // eat the sym key.
        if (keyCode == SymKey) {
            eaten = true;
        }
        return eaten;
    }

    // When Fn is pressed while also being locked, fn modifier is ignored for obtaining the text
    if ((keyCode != FnLevelKey) && fnPressed && (currentLockedMods & FnModifierMask)) {
        text = keycodeToString(nativeScanCode, (nativeModifiers & ShiftMask) ? 1 : 0);
        keyCode = text.isEmpty() ? Qt::Key_unknown : static_cast<Qt::Key>(QKeySequence(text)[0]);
    }

    const unsigned char savedLatchedMods(currentLatchedMods);
    pressedKeys.insert(nativeScanCode, true);

    if (keyCode == SymKey) {
        characterLoopIndex = -1;
    } else if ((keyCode == Qt::Key_Shift) && (++shiftsPressed == 2)
               && !stateTransitionsDisabled) {
        shiftShiftCapsLock = true;
        latchModifiers(FnModifierMask | ShiftMask, 0);
        lockModifiers(FnModifierMask | LockMask, LockMask);
    } else if (keyCode == FnLevelKey) {
        fnPressed = true;
        eaten = false;
    } else {
        eaten = !passKeyOnPress(keyCode, text, nativeScanCode);

        // Long press feature, only applies for the latest keypress (i.e. the latest
        // keypress event cancels the long press logic for the previous keypress)
        if (eaten) {
            longPressKey = nativeScanCode;
            longPressModifiers = nativeModifiers;
            longPressTimer.start();
        }
        // Unlatch modifiers on keys for which there is a known action on press event.
        // Currently this means arrow keys and backspace/delete.
        if (!eaten && !autoCaps && currentLatchedMods && actionOnPress(keyCode)) {
            latchModifiers(FnModifierMask | ShiftMask, 0);
        }
    }

    // Shift modifier can be on even when shift is not held down, namely when shift is
    // latched.  This breaks MTextEdit's shift+arrow-key selection, which is specified to
    // work only when shift is held down, because it works by looking at the shift
    // modifier.  This also breaks standard Qt copy/paste/selectall shortcuts.  So, we
    // mask shift modifier away in the case of latched shift.  We also turn delete event
    // generated with latched shift and backspace key into backspace event.
    if (!eaten && (savedLatchedMods & ShiftMask) && !shiftsPressed) {
        inputContextConnection.sendKeyEvent(
            QKeyEvent(QEvent::KeyPress, keyCode == Qt::Key_Delete ? Qt::Key_Backspace : keyCode,
                      modifiers & ~Qt::KeyboardModifiers(Qt::ShiftModifier),
                      keyCode == Qt::Key_Delete ? "\b" : text, autoRepeat, count));
        eaten = true;
    }
    // Delete is generated by pressing Shift+Backspace.  This results to an event with
    // shift modifier and Delete key code, which can be interpreted as the cut shortcut
    // even though user intended just delete action.  Shift+Del shouldn't be cut shortcut
    // on our platform but Qt is not going to change that, so we work around the problem
    // by removing the shift modifier.
    else if (shiftsPressed && (keyCode == Qt::Key_Delete)) {
        inputContextConnection.sendKeyEvent(
            QKeyEvent(QEvent::KeyPress, keyCode,
                      modifiers & ~Qt::KeyboardModifiers(Qt::ShiftModifier),
                      text, autoRepeat, count));
        eaten = true;
    }

    // Relatch modifiers, X unlatches them on press but we want to unlatch on release
    // (and not even always on release, e.g. with Sym+aaab we unlatch only after the
    // last "a").
    if (currentLatchedMods) {
        mXkb.latchModifiers(currentLatchedMods, currentLatchedMods);
    }

    return eaten;
}

bool MHardwareKeyboard::filterKeyRelease(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                                         QString text,
                                         quint32 nativeScanCode, quint32 nativeModifiers)
{
    bool eaten = false;

    // switch keyboard map if Fn and Shift are held down and one of them
    // released (without pressing any character key)
    if(keyCode == Qt::Key_Shift
        && fnPressed
        && (lastKeyCode == FnLevelKey || lastKeyCode == Qt::Key_Shift)) {
        --shiftsPressed;
        switchKeyMap();
        return true;
    }
    if (keyCode == FnLevelKey
        && shiftsPressed
        && (lastKeyCode == FnLevelKey || lastKeyCode == Qt::Key_Shift)) {
        fnPressed = false;
        switchKeyMap();
        return true;
    }
    // eat Space release when Control is held.
    if (keyCode == Qt::Key_Space
        && (modifiers & Qt::ControlModifier)
        && (!fnPressed && !shiftsPressed)) {
        return true;
    }

    if (imMode == M::InputMethodModeDirect) {
        if (keyCode == SymKey) {
            eaten = handleReleaseWithSymModifier(keyCode, text);
        }
        return eaten;
    }

    // When Fn is pressed while also being locked, fn modifier is ignored for obtaining the text
    if ((keyCode != FnLevelKey) && fnPressed && (currentLockedMods & FnModifierMask)) {
        text = keycodeToString(nativeScanCode, (nativeModifiers & ShiftMask) ? 1 : 0);
        keyCode = text.isEmpty() ? Qt::Key_unknown : static_cast<Qt::Key>(QKeySequence(text)[0]);
    }
    const bool keyWasPressed(pressedKeys.contains(nativeScanCode));

    // Relock modifiers; X seems to unlock Fn automatically on Fn release, which is not
    // desired at least with [phone] number content type.
    if (currentLockedMods) {
        mXkb.lockModifiers(currentLockedMods, currentLockedMods);
    }

    if (keyWasPressed && (nativeModifiers & SymModifierMask)) {
        eaten = handleReleaseWithSymModifier(keyCode, text);
    }

    const Qt::KeyboardModifiers filteredModifiers(
        ((currentLatchedMods & ShiftMask) && !shiftsPressed)
        ? (modifiers & ~Qt::KeyboardModifiers(Qt::ShiftModifier))
        : modifiers);

    if (keyCode == Qt::Key_Shift) {
        if (!shiftShiftCapsLock) {
            handleCyclableModifierRelease(Qt::Key_Shift, LockMask, ShiftMask, FnModifierMask,
                                          FnModifierMask);
        }
        if (--shiftsPressed == 0) {
            shiftShiftCapsLock = false;
        }
    } else if (keyCode == FnLevelKey) {
        fnPressed = false;
        handleCyclableModifierRelease(FnLevelKey, FnModifierMask, FnModifierMask, LockMask,
                                      ShiftMask);
    } else if (!eaten && !passKeyOnPress(keyCode, text, nativeScanCode)) {
        if (keyWasPressed) {
            inputContextConnection.sendKeyEvent(
                QKeyEvent(QEvent::KeyPress, keyCode, filteredModifiers, text, false, 1));
            inputContextConnection.sendKeyEvent(
                QKeyEvent(QEvent::KeyRelease, keyCode, filteredModifiers, text, false, 1));
        }
        eaten = true;

        if (!autoCaps) {
            latchModifiers(FnModifierMask | ShiftMask, 0);
        }
    }
    // This case works around the problem of host calling setAutoCapitalization()
    // after backspace press event but before backspace release.  That turns the
    // release event into a Delete release!
    else if (!shiftsPressed && (keyCode == Qt::Key_Delete)) {
        inputContextConnection.sendKeyEvent(
            QKeyEvent(QEvent::KeyRelease, Qt::Key_Backspace, filteredModifiers, "\b", false, 1));
        eaten = true;
    }
    // See the same case in filterKeyPress
    else if (shiftsPressed && (keyCode == Qt::Key_Delete)) {
        inputContextConnection.sendKeyEvent(
            QKeyEvent(QEvent::KeyRelease, keyCode,
                      modifiers & ~Qt::KeyboardModifiers(Qt::ShiftModifier), text, false, 1));
        eaten = true;
    }

    pressedKeys.remove(nativeScanCode);

    return eaten;
}

bool MHardwareKeyboard::filterKeyEvent(QEvent::Type eventType,
                                       Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                                       const QString &text, bool autoRepeat, int count,
                                       quint32 nativeScanCode, quint32 nativeModifiers)
{
    XkbStateRec xkbState;
    XkbGetState(QX11Info::display(), XkbUseCoreKbd, &xkbState); // TODO: XkbUseCoreKbd

    bool eaten = false;

    if (eventType == QEvent::KeyPress) {
        eaten = filterKeyPress(keyCode, modifiers, text, autoRepeat, count,
                               nativeScanCode, nativeModifiers);
    } else {
        eaten = filterKeyRelease(keyCode, modifiers, text, nativeScanCode, nativeModifiers);
    }

    lastEventType = eventType;
    lastKeyCode = keyCode;

    return eaten;
}


QString MHardwareKeyboard::keycodeToString(unsigned int keycode, unsigned int shiftLevel) const
{
    const unsigned int group(0);
    KeySym keySym(XkbKeycodeToKeysym(QX11Info::display(), static_cast<KeyCode>(keycode),
                                     group, shiftLevel));
    if (keySym == NoSymbol) {
        return QString();
    }
    const int stringBufferSize(8);
    char stringBuffer[stringBufferSize];
    int stringBufferOverflow;
    const unsigned int modifiers(0);
    const int stringLength(XkbTranslateKeySym(QX11Info::display(), &keySym, modifiers,
                                              stringBuffer, stringBufferSize, &stringBufferOverflow));

    if (stringBufferOverflow) {
        qWarning() << "Unable to convert keycode" << keycode
                   << "to string with shift level" << shiftLevel << ": insufficient buffer.";
        return QString();
    }

    const QString text(QTextCodec::codecForLocale()->toUnicode(stringBuffer, stringLength));
    return text;
}


void MHardwareKeyboard::handleLongPressTimeout()
{
    if (!pressedKeys.contains(longPressKey)) {
        return;
    }

    const unsigned int shiftAndLock(longPressModifiers & (ShiftMask | LockMask));
    // With Fn locked, long press overrides Fn modifier and you'll get level 0 or 1 key
    const unsigned int shiftLevelBase(currentLockedMods & FnModifierMask ? 0 : 2);
    const unsigned int shiftLevel(((shiftAndLock == ShiftMask) || (shiftAndLock == LockMask)
                                   ? 1 : 0) + shiftLevelBase);
    const QString text(keycodeToString(longPressKey, shiftLevel));
    if (!text.isEmpty()) {
        inputContextConnection.sendKeyEvent(
            QKeyEvent(QEvent::KeyPress, static_cast<Qt::Key>(QKeySequence(text)[0]), Qt::NoModifier,
                      text, false, 1));
        inputContextConnection.sendKeyEvent(
            QKeyEvent(QEvent::KeyRelease, static_cast<Qt::Key>(QKeySequence(text)[0]), Qt::NoModifier,
                      text, false, 1));
        latchModifiers(FnModifierMask | ShiftMask, 0);
        pressedKeys.remove(longPressKey);
    }
}


bool MHardwareKeyboard::handleReleaseWithSymModifier(Qt::Key keyCode, const QString &text)
{
    if ((imMode == M::InputMethodModeDirect) && (keyCode == SymKey)) {
        emit symbolKeyClicked();
        return true;
    }

    //FIXME: eat the sym key?
    if ((lastKeyCode == SymKey) && (keyCode == SymKey)) {
        emit symbolKeyClicked();
        return false;
    }

    if ((characterLoopIndex != -1) && ((lastSymText != text) || (keyCode == SymKey))) {
        const QString accentedCharacters = hwkbCharLoopsManager.characterLoop(lastSymText[0]);
        inputContextConnection.sendCommitString(QString(accentedCharacters[characterLoopIndex]));
        characterLoopIndex = -1;
        latchModifiers(FnModifierMask | ShiftMask, 0);
        // TODO: sym+character with latched shift.  Also note the return false cases.
    }

    if (text.length() != 1) {
        return false;
    }

    const QString accentedCharacters = hwkbCharLoopsManager.characterLoop(text[0]);
    if (accentedCharacters.isEmpty()) {
        return false;
    }

    lastSymText = text;
    characterLoopIndex = (characterLoopIndex + 1) % accentedCharacters.length();
    inputContextConnection.sendPreeditString(accentedCharacters[characterLoopIndex],
                                             PreeditNoCandidates);
    return true;
}


void MHardwareKeyboard::setAutoCapitalization(bool state)
{
    if (autoCaps != state) {
        // Set auto caps when there is no custom state being set for shift/fn modifier.
        // Also ignore host's attempts to set autocaps to false while Sym+c looping is in
        // progress.
        if (((((currentLockedMods & (FnModifierMask | LockMask)) == 0)
              && ((currentLatchedMods & (FnModifierMask | ShiftMask)) == 0))
             || autoCaps)
            && !stateTransitionsDisabled && (characterLoopIndex == -1)) {
            latchModifiers(ShiftMask, state ? ShiftMask : 0);
            autoCaps = state;
        }
    }
}


ModifierState MHardwareKeyboard::modifierState(Qt::KeyboardModifier modifier) const
{
    unsigned int latchMask = 0;
    unsigned int lockMask = 0;

    if (modifier == Qt::ShiftModifier) {
        latchMask = ShiftMask;
        lockMask = LockMask;
    } else if (modifier == FnLevelModifier) {
        latchMask = lockMask = FnModifierMask;
    }

    if (currentLatchedMods & latchMask) {
        return ModifierLatchedState;
    } else if (currentLockedMods & lockMask) {
        return ModifierLockedState;
    } else {
        return ModifierClearState;
    }
}


bool MHardwareKeyboard::symViewAvailable() const
{
    return (currentKeyboardType != M::NumberContentType)
        && (currentKeyboardType != M::PhoneNumberContentType);
}

bool MHardwareKeyboard::autoCapsEnabled() const
{
    return LayoutsManager::instance().hardwareKeyboardAutoCapsEnabled();
}

void MHardwareKeyboard::setInputMethodMode(M::InputMethodMode mode)
{
    imMode = mode;
}

M::InputMethodMode MHardwareKeyboard::inputMethodMode() const
{
    return imMode;
}

void MHardwareKeyboard::switchKeyMap()
{
    qDebug() << __PRETTY_FUNCTION__;
    const QString secondaryLayout = LayoutsManager::instance().xkbSecondaryLayout();
    const QString secondaryVariant = LayoutsManager::instance().xkbSecondaryVariant();
    // only switches layout when current xkb map supports secondary layout.
    if (!secondaryLayout.isEmpty()) {
        const QString currentLayout = LayoutsManager::instance().xkbLayout();
        const QString currentVariant = LayoutsManager::instance().xkbVariant();

        QString nextLayout, nextVariant;
        if (currentLayout == secondaryLayout
            && currentVariant == secondaryVariant) {
            nextLayout = LayoutsManager::instance().xkbPrimaryLayout();
            nextVariant = LayoutsManager::instance().xkbPrimaryVariant();
        } else {
            nextLayout = secondaryLayout;
            nextVariant = secondaryVariant;
        }
        if (mXkb.setXkbMap(LayoutsManager::instance().xkbModel(), nextLayout, nextVariant)) {
            LayoutsManager::instance().setXkbMap(nextLayout, nextVariant);
        }
    }
}
