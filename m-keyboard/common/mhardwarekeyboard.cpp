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
#include "mvirtualkeyboard.h"

#include <X11/X.h>
#undef KeyPress
#undef KeyRelease
#include <X11/XKBlib.h>

#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

namespace
{
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    const Qt::Key FnLevelKey = Qt::Key_AltGr;
    const Qt::Key SymKey = Qt::Key_Multi_key;
    const unsigned int SymModifierMask = Mod4Mask;
    const unsigned int FnModifierMask = Mod5Mask;
    const unsigned int longPressTime = 600; // in milliseconds
};

MHardwareKeyboard::MHardwareKeyboard(MInputContextConnection& icConnection, QObject *parent)
    : QObject(parent),
      keyboardType(M::FreeTextContentType),
      autoCaps(false),
      inputContextConnection(icConnection),
      lastEventType(QEvent::KeyRelease),
      currentLatchedMods(0),
      currentLockedMods(0),
      characterLoopIndex(-1),
      stateTransitionsDisabled(false),
      shiftsPressed(0),
      shiftShiftCapsLock(false),
      longPressTimer(this)
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
    keyboardType = type;

    switch (keyboardType) {
    case M::NumberContentType:
    case M::PhoneNumberContentType:
        // With number and phone number content type Fn must be permanently locked
        lockModifiers(FnModifierMask, FnModifierMask);
        stateTransitionsDisabled = true;
        break;
    default:
        stateTransitionsDisabled = false;
        break;
    }
}


void MHardwareKeyboard::reset()
{
    qDebug() << __PRETTY_FUNCTION__;
    // TODO: this is a temporary hack until we have proper autorepeat setup
    XAutoRepeatOff(QX11Info::display());

    shiftShiftCapsLock = false;
    shiftsPressed = 0;
    pressedKeys.clear();
    currentLatchedMods = 0;
    currentLockedMods = 0;
    autoCaps = false;
    mXkb.lockModifiers(LockMask | FnModifierMask, 0);
    mXkb.latchModifiers(ShiftMask | FnModifierMask, 0);
    emit modifierStateChanged(Qt::ShiftModifier, ModifierClearState);
    emit modifierStateChanged(FnLevelModifier, ModifierClearState);
}


bool MHardwareKeyboard::passKeyOnPress(Qt::Key keyCode, const QString &text,
                                       unsigned int nativeScanCode) const
{
    static const Qt::Key pressPassKeys[] = {
        Qt::Key_Backspace, Qt::Key_Delete };
    static const Qt::Key * const keysEnd = pressPassKeys + ELEMENTS(pressPassKeys);

    const unsigned int shiftLevel(3); // Fn.
    return (text.isEmpty() && keycodeToString(nativeScanCode, shiftLevel).isEmpty())
        || keysEnd != std::find(pressPassKeys, keysEnd, keyCode);
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
                                       const QString &text, bool autoRepeat, int count,
                                       quint32 nativeScanCode, quint32 nativeModifiers)
{
    bool eaten = false;
    pressedKeys.insert(nativeScanCode, true);

    if (keyCode == SymKey) {
        characterLoopIndex = -1;
    } else if ((keyCode == Qt::Key_Delete) && (currentLatchedMods & ShiftMask)
               && !shiftsPressed) {
        inputContextConnection.sendKeyEvent(
            QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace,
                      modifiers & ~Qt::KeyboardModifiers(Qt::ShiftModifier),
                      "\b", autoRepeat, count));
        eaten = true;
    }
    // MTextEdit implements shift+arrow-key selection by simply looking at the shift
    // modifier in key events it gets.  However, shift+arrow-key is specified to work only
    // when shift is held down and shift modifier can be on even when shift is not held
    // down, namely when shift is latched.  It would be tricky for MTextEdit to track
    // shift [not] pressed state across MTextEdit instances so we work around the
    // situation by removing shift modifer when shift is latched but not pressed.
    else if (((keyCode == Qt::Key_Left) || (keyCode == Qt::Key_Right) || (keyCode == Qt::Key_Up)
              || (keyCode == Qt::Key_Down))
             && (currentLatchedMods & ShiftMask) && !shiftsPressed) {
        inputContextConnection.sendKeyEvent(
            QKeyEvent(QEvent::KeyPress, keyCode,
                      modifiers & ~Qt::KeyboardModifiers(Qt::ShiftModifier),
                      text, autoRepeat, count));
        eaten = true;
    } else if ((keyCode == Qt::Key_Shift) && (++shiftsPressed == 2)
               && !stateTransitionsDisabled) {
        shiftShiftCapsLock = true;
        latchModifiers(FnModifierMask | ShiftMask, 0);
        lockModifiers(FnModifierMask | LockMask, LockMask);
    } else {
        eaten = !passKeyOnPress(keyCode, text, nativeScanCode);

        // Long press feature, only applies for the latest keypress (i.e. the latest
        // keypress event cancels the long press logic for the previous keypress)
        if (eaten) {
            longPressKey = nativeScanCode;
            longPressModifiers = nativeModifiers;
            longPressTimer.start();
        }
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
                                         const QString &text,
                                         quint32 nativeScanCode, quint32 nativeModifiers)
{
    bool eaten = false;

    if (nativeModifiers & SymModifierMask) {
        eaten = handleReleaseWithSymModifier(keyCode, text);
    }

    if (keyCode == Qt::Key_Shift) {
        if (!shiftShiftCapsLock) {
            handleCyclableModifierRelease(Qt::Key_Shift, LockMask, ShiftMask, FnModifierMask,
                                          FnModifierMask);
        }
        if (--shiftsPressed == 0) {
            shiftShiftCapsLock = false;
        }
    } else if (keyCode == FnLevelKey) {
        handleCyclableModifierRelease(FnLevelKey, FnModifierMask, FnModifierMask, LockMask,
                                      ShiftMask);
    } else if (!eaten && !passKeyOnPress(keyCode, text, nativeScanCode)) {
        const bool keyWasPressed(pressedKeys.contains(nativeScanCode));
        if (keyWasPressed) {
            inputContextConnection.sendKeyEvent(
                QKeyEvent(QEvent::KeyPress, keyCode, modifiers, text, false, 1));
            // TODO: should we send release too?  MTextEdit seems to be happy with
            // just press but what about others?
            eaten = true;
        }

        latchModifiers(FnModifierMask | ShiftMask, 0);
    }
    // This case works around the problem of host calling setAutoCapitalization()
    // after backspace press event but before backspace release.  That turns the
    // release event into a Delete release!  We eat backspace releases for consistency
    // as well.  If the answer the the above "should we send release too?" question is
    // "yes", presumably we need other kind of trickery here.
    else if ((keyCode == Qt::Key_Backspace) || (keyCode == Qt::Key_Delete)) {
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
    const unsigned int shiftLevel((shiftAndLock == ShiftMask) || (shiftAndLock == LockMask)
                                  ? 3 : 2);
    const QString text(keycodeToString(longPressKey, shiftLevel));
    if (!text.isEmpty()) {
        inputContextConnection.sendKeyEvent(
            QKeyEvent(QEvent::KeyPress, static_cast<Qt::Key>(QKeySequence(text)[0]), Qt::NoModifier,
                      text, false, 1));
        latchModifiers(FnModifierMask | ShiftMask, 0);
        pressedKeys.remove(longPressKey);
    }
}


bool MHardwareKeyboard::handleReleaseWithSymModifier(Qt::Key keyCode, const QString &text)
{
    if ((lastKeyCode == SymKey) && (keyCode == SymKey)) {
        emit symbolKeyClicked();
        return true;            // TODO: or false?
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
    return (keyboardType != M::NumberContentType)
        && (keyboardType != M::PhoneNumberContentType);
}
