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
#include <QApplication>
#include <QClipboard>
#include <algorithm>

#include <mplainwindow.h>
#include <MSceneWindow>
#include <MBanner>
#include <mabstractinputmethodhost.h>

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
    const int CopyValidForNotificationInterval(500); // in ms
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    const Qt::Key FnLevelKey = Qt::Key_AltGr;
    const Qt::Key SymKey = Qt::Key_Multi_key;
    const unsigned int SymModifierMask = Mod4Mask;
    const unsigned int FnModifierMask = Mod5Mask;
    const unsigned int longPressTime = 600; // in milliseconds

    const unsigned short RepeatDelay(600);   // in milliseconds
    // Repeat rate is specified (at least as of this writing) to be slightly different for
    // horizontal and vertical operations (lower) but we can't have key-specific repeat
    // rates if we want to keep using X's autorepeat.  So we use the repeat rate of
    // horizontal operation keys for all keys for now.
    const unsigned short RepeatInterval(100); // in milliseconds

    //! Character set for MTextEdit number content type
    const QString NumberContentCharacterSetRegexp(QString("[-+0-9%1-%2,.%3]")
                                                  .arg(QChar(0x0660)) // Arabic numbers begin
                                                  .arg(QChar(0x0669)) // Arabic numbers end
                                                  .arg(QChar(0x066b))); // Arabic decimal separator

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

MHardwareKeyboard::MHardwareKeyboard(MAbstractInputMethodHost& imHost, QObject *parent)
    : QObject(parent),
      currentKeyboardType(M::FreeTextContentType),
      autoCaps(false),
      inputMethodHost(imHost),
      lastEventType(QEvent::KeyRelease),
      currentLatchedMods(0),
      currentLockedMods(0),
      characterLoopIndex(-1),
      stateTransitionsDisabled(false),
      shiftsPressed(0),
      shiftShiftCapsLock(false),
      longPressTimer(this),
      imMode(M::InputMethodModeNormal),
      fnPressed(false),
      numberContentCharacterMatcher(NumberContentCharacterSetRegexp)
{
    longPressTimer.setSingleShot(true);
    longPressTimer.setInterval(longPressTime);
    connect(&longPressTimer, SIGNAL(timeout()), this, SLOT(handleLongPressTimeout()));
    connect(&deadKeyMapper, SIGNAL(stateChanged(const QChar &)),
            this, SIGNAL(deadKeyStateChanged(const QChar &)));
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
    latchModifiers(LockMask | FnModifierMask, 0);
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

    inputMethodHost.setDetectableAutoRepeat(enable);
}

void MHardwareKeyboard::enable()
{
    qDebug() << __PRETTY_FUNCTION__;

    if (imMode != M::InputMethodModeDirect) {
        connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(handleClipboardDataChange()));
        toggleCustomAutoRepeat(true);

        shiftShiftCapsLock = false;
        shiftsPressed = 0;
        pressedKeys.clear();
        fnPressed = false;
        autoCaps = false;

        preedit.clear();
        deadKeyMapper.reset();

        // We reset state without using latch/lockModifiers in order to force notification
        // (to make sure whoever is listening is in sync with us).
        currentLatchedMods = 0;
        mXkb.latchModifiers(LockMask | FnModifierMask, 0);
        switch (currentKeyboardType) {
        case M::NumberContentType:
        case M::PhoneNumberContentType:
            // With number and phone number content type Fn must be permanently locked
            currentLockedMods = FnModifierMask;
            mXkb.lockModifiers(FnModifierMask, FnModifierMask);
            stateTransitionsDisabled = true;
            break;
        default:
            stateTransitionsDisabled = false;
            // clear locked modifiers for other keyboard types.
            currentLockedMods = 0;
            mXkb.lockModifiers(LockMask | FnModifierMask, 0);
            break;
        }
        emit modifiersStateChanged();
    }

    inputMethodHost.setRedirectKeys(true);
}


void MHardwareKeyboard::disable()
{
    qDebug() << __PRETTY_FUNCTION__;

    disconnect(QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);

    if (!preedit.isEmpty()) {
        inputMethodHost.sendPreeditString("", MInputMethod::PreeditKeyPress);
        preedit.clear();
    }
    deadKeyMapper.reset();

    inputMethodHost.setRedirectKeys(false);
    // Unlock and unlatch everything.  If for example non-Qt X application gets focus
    // after this focus out, there is no way to unlock Lock modifier using the
    // physical keyboard.  So better clear the state as well as we can.
    lockModifiers(LockMask | FnModifierMask, 0);
    latchModifiers(LockMask | FnModifierMask, 0);

    toggleCustomAutoRepeat(false);
}


void MHardwareKeyboard::reset()
{
    qDebug() << __PRETTY_FUNCTION__;

    deadKeyMapper.reset();
    preedit.clear();
}


void MHardwareKeyboard::clientChanged()
{
    reset();
}


void MHardwareKeyboard::handleClipboardDataChange()
{
    if (!lastCtrlCTime.isValid()
        || (lastCtrlCTime.addMSecs(CopyValidForNotificationInterval) < QTime::currentTime())) {
        lastCtrlCTime = QTime();
        return;
    }
    lastCtrlCTime = QTime();

    MBanner &textCopiedBanner(*new MBanner);
    textCopiedBanner.setStyleName("InformationBanner");
    textCopiedBanner.setTitle(qtTrId("qtn_comm_text_copied"));
    textCopiedBanner.appear(MPlainWindow::instance(), MSceneWindow::DestroyWhenDone);
}


bool MHardwareKeyboard::actionOnPress(Qt::Key keyCode) const
{
    static const Qt::Key pressPassKeys[] = {
        Qt::Key_Return, Qt::Key_Backspace, Qt::Key_Delete,
        Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down,
        Qt::Key_Home, Qt::Key_End, Qt::Key_PageUp, Qt::Key_PageDown };
    static const Qt::Key * const keysEnd = pressPassKeys + ELEMENTS(pressPassKeys);

    return keysEnd != std::find(pressPassKeys, keysEnd, keyCode);
}

bool MHardwareKeyboard::passKeyOnPress(Qt::Key keyCode, const QString &text,
                                       quint32 nativeScanCode, quint32 nativeModifiers) const
{
    const unsigned int shiftLevel(3); // Fn.
    return (text.isEmpty() && keycodeToString(nativeScanCode, shiftLevel).isEmpty())
        || actionOnPress(keyCode) || (nativeModifiers & ControlMask);
}


void MHardwareKeyboard::notifyModifierChange(unsigned char previousModifiers,
                                             unsigned int shiftMask,
                                             unsigned int affect,
                                             unsigned int value) const
{
    if ((affect & shiftMask) && ((value & shiftMask) != (previousModifiers & shiftMask))) {
        emit shiftStateChanged();
        emit modifiersStateChanged();
    }
    if ((affect & FnModifierMask)
        && ((value & FnModifierMask) != (previousModifiers & FnModifierMask))) {
        emit modifiersStateChanged();
    }
}


void MHardwareKeyboard::latchModifiers(unsigned int affect, unsigned int value)
{
    mXkb.latchModifiers(affect, value);
    const unsigned int savedLatchedMods = currentLatchedMods;
    currentLatchedMods = (currentLatchedMods & ~affect) | (value & affect);
    if (!(currentLatchedMods & LockMask)) {
        autoCaps = false;
    }
    notifyModifierChange(savedLatchedMods, LockMask, affect, value);
}


void MHardwareKeyboard::lockModifiers(unsigned int affect, unsigned int value)
{
    mXkb.lockModifiers(affect, value);
    const unsigned int savedLockedMods = currentLockedMods;
    currentLockedMods = (currentLockedMods & ~affect) | (value & affect);
    notifyModifierChange(savedLockedMods, LockMask, affect, value);
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

bool MHardwareKeyboard::handleScriptSwitchOnPress(Qt::Key keyCode, Qt::KeyboardModifiers modifiers)
{
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

    return false;
}

bool MHardwareKeyboard::handleScriptSwitchOnRelease(Qt::Key keyCode, Qt::KeyboardModifiers modifiers)
{
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

    return false;
}

void MHardwareKeyboard::correctToAcceptedCharacter(QString &text, const quint32 nativeScanCode,
                                                   quint32 &nativeModifiers) const
{
    if ((currentKeyboardType != M::NumberContentType)
        || numberContentCharacterMatcher.exactMatch(text)) {
        return;
    }

    // It's sufficient for us to consider only the first level since all we really want to
    // do is make it possible to enter decimal separators without long press or pressing
    // Fn.
    const QString noFnCandidate(keycodeToString(nativeScanCode, 0));
    if (numberContentCharacterMatcher.exactMatch(noFnCandidate)) {
        text = noFnCandidate;
        nativeModifiers &= ~FnModifierMask;
    }
}

bool MHardwareKeyboard::filterArrowKeys(QEvent::Type eventType, Qt::Key keyCode,
                                        Qt::KeyboardModifiers modifiers,
                                        QString text, bool autoRepeat, int count,
                                        quint32 nativeModifiers) const
{
    // Home/End/PageUp/PageDown only when user is holding Fn down
    if (!fnPressed && (nativeModifiers & FnModifierMask)
        && (keyCode <= Qt::Key_PageDown) && (keyCode >= Qt::Key_Home)) {
        Q_ASSERT(text.isEmpty());
        switch (keyCode) {
        case Qt::Key_Home:
            keyCode = Qt::Key_Left;
            break;
        case Qt::Key_End:
            keyCode = Qt::Key_Right;
            break;
        case Qt::Key_PageUp:
            keyCode = Qt::Key_Up;
            break;
        case Qt::Key_PageDown:
            keyCode = Qt::Key_Down;
            break;
        default:
            Q_ASSERT_X(false, "MHardwareKeyboard::filterArrowKeys",
                       "Left, Right, Up or Down with Fn modifier");
            break;
        }
        inputMethodHost.sendKeyEvent(
            QKeyEvent(eventType, keyCode, modifiers, text, autoRepeat, count),
            MInputMethod::EventRequestEventOnly);
        return true;
    }

    return false;
}

bool MHardwareKeyboard::filterKeyPress(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                                       QString text, bool autoRepeat, int count,
                                       quint32 nativeScanCode, quint32 nativeModifiers)
{
    bool eaten = false;

    if (handleScriptSwitchOnPress(keyCode, modifiers)) {
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
    // When shift is latched (using lock modifier), shift modifier must not reverse the
    // effect of Lock
    else if ((currentLatchedMods & LockMask) && (nativeModifiers & ShiftMask)
             && (keyCode != Qt::Key_Shift) && (text.length() == 1) && text[0].isLetter()) {
        text = keycodeToString(nativeScanCode, (nativeModifiers & FnModifierMask) ? 3 : 1);
        keyCode = text.isEmpty() ? Qt::Key_unknown : static_cast<Qt::Key>(QKeySequence(text)[0]);
    }

    pressedKeys.insert(nativeScanCode, nativeModifiers);

    if (keyCode == SymKey) {
        characterLoopIndex = -1;
        eaten = true;
    } else if ((keyCode == Qt::Key_Shift) && (++shiftsPressed == 2)
               && !stateTransitionsDisabled) {
        shiftShiftCapsLock = true;
        latchModifiers(FnModifierMask | LockMask, 0);
        lockModifiers(FnModifierMask | LockMask, LockMask);
        eaten = true;
    } else if (keyCode == Qt::Key_Shift) {
        eaten = true;
    } else if (keyCode == FnLevelKey) {
        fnPressed = true;
        eaten = true;
    } else {
        eaten = !passKeyOnPress(keyCode, text, nativeScanCode, nativeModifiers);
        bool eatenBySymHandler(false);

        if (nativeModifiers & SymModifierMask) {
            eatenBySymHandler = handlePressWithSymModifier(text, nativeScanCode, nativeModifiers);
            eaten = eaten || eatenBySymHandler;
        } else if ((nativeModifiers & ControlMask) && (keyCode == Qt::Key_C)) {
            lastCtrlCTime.start();
        }

        if (eaten && !eatenBySymHandler) {
            correctToAcceptedCharacter(text, nativeScanCode, nativeModifiers);

            if (!preedit.isEmpty() && (preedit != deadKeyMapper.currentDeadKey())) {
                inputMethodHost.sendCommitString(preedit);
                pressedKeys.remove(preeditScanCode);
            }

            if (!deadKeyMapper.filterKeyPress(text)) {
                // Long press feature, only applies for the latest keypress (i.e. the latest
                // keypress event cancels the long press logic for the previous keypress)
                longPressKey = nativeScanCode;
                longPressModifiers = nativeModifiers;
                longPressTimer.start();
            }
            inputMethodHost.sendPreeditString(text, MInputMethod::PreeditKeyPress);
            preedit = text;
            preeditScanCode = nativeScanCode;
        }
        // Unlatch modifiers on keys for which there is a known action on press event.
        // Currently this means arrow keys and backspace/delete.
        if (!eaten && !autoCaps && currentLatchedMods && actionOnPress(keyCode)) {
            latchModifiers(FnModifierMask | LockMask, 0);
        }
    }

    // Delete is generated by pressing Shift+Backspace.  This results to an event with
    // shift modifier and Delete key code, which can be interpreted as the cut shortcut
    // even though user intended just delete action.  Shift+Del shouldn't be cut shortcut
    // on our platform but Qt is not going to change that, so we work around the problem
    // by removing the shift modifier.
    if (shiftsPressed && (keyCode == Qt::Key_Delete)) {
        inputMethodHost.sendKeyEvent(
            QKeyEvent(QEvent::KeyPress, keyCode,
                      modifiers & ~Qt::KeyboardModifiers(Qt::ShiftModifier),
                      text, autoRepeat, count),
            MInputMethod::EventRequestEventOnly);
        eaten = true;
    } else if (!eaten){
        eaten = filterArrowKeys(QEvent::KeyPress, keyCode, modifiers, text, autoRepeat, count,
                                nativeModifiers);
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

    if (handleScriptSwitchOnRelease(keyCode, modifiers)) {
        return true;
    }

    if (imMode == M::InputMethodModeDirect) {
        if (keyCode == SymKey) {
            eaten = handleReleaseWithSymModifier(keyCode);
        }
        return eaten;
    }

    // When Fn is pressed while also being locked, fn modifier is ignored for obtaining the text
    if ((keyCode != FnLevelKey) && fnPressed && (currentLockedMods & FnModifierMask)) {
        text = keycodeToString(nativeScanCode, (nativeModifiers & ShiftMask) ? 1 : 0);
        keyCode = text.isEmpty() ? Qt::Key_unknown : static_cast<Qt::Key>(QKeySequence(text)[0]);
    }
    const bool keyWasPressed(pressedKeys.contains(nativeScanCode));
    const quint32 pressNativeModifiers(pressedKeys.value(nativeScanCode));

    // Relock modifiers; X seems to unlock Fn automatically on Fn release, which is not
    // desired at least with [phone] number content type.
    if (currentLockedMods) {
        mXkb.lockModifiers(currentLockedMods, currentLockedMods);
    }

    if (keyWasPressed && (nativeModifiers & SymModifierMask)) {
        eaten = handleReleaseWithSymModifier(keyCode);
    }

    if (keyCode == Qt::Key_Shift) {
        if (!shiftShiftCapsLock) {
            handleCyclableModifierRelease(Qt::Key_Shift, LockMask, LockMask, FnModifierMask,
                                          FnModifierMask);
        }
        if (--shiftsPressed == 0) {
            shiftShiftCapsLock = false;
        }
        eaten = true;
    } else if (keyCode == FnLevelKey) {
        fnPressed = false;
        handleCyclableModifierRelease(FnLevelKey, FnModifierMask, FnModifierMask, LockMask,
                                      LockMask);
        eaten = true;
    } else if (!eaten && !passKeyOnPress(keyCode, text, nativeScanCode, nativeModifiers)
               && !(pressNativeModifiers & ControlMask)) {
        const bool deadKey(preedit == deadKeyMapper.currentDeadKey());

        if (keyWasPressed && !deadKey) {
            inputMethodHost.sendCommitString(preedit);
            preedit.clear();
        }
        eaten = true;

        if (!autoCaps && !deadKey && !(pressNativeModifiers & ShiftMask)) {
            latchModifiers(FnModifierMask | LockMask, 0);
        }
    }
    // See the same case in filterKeyPress
    else if (shiftsPressed && (keyCode == Qt::Key_Delete)) {
        inputMethodHost.sendKeyEvent(
            QKeyEvent(QEvent::KeyRelease, keyCode,
                      modifiers & ~Qt::KeyboardModifiers(Qt::ShiftModifier), text, false, 1),
            MInputMethod::EventRequestEventOnly);
        eaten = true;
    } else if (!eaten) {
        eaten = filterArrowKeys(QEvent::KeyRelease, keyCode, modifiers, text, false, 1,
                                nativeModifiers);
    }

    pressedKeys.remove(nativeScanCode);

    return eaten;
}

bool MHardwareKeyboard::filterKeyEvent(QEvent::Type eventType,
                                       Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                                       const QString &text, bool autoRepeat, int count,
                                       quint32 nativeScanCode, quint32 nativeModifiers)
{
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
    QString text(keycodeToString(longPressKey, shiftLevel));
    if (!text.isEmpty()) {
        (void)deadKeyMapper.filterKeyPress(text, true);
        preedit = text;
        inputMethodHost.sendPreeditString(text, MInputMethod::PreeditKeyPress);
    }
}


void MHardwareKeyboard::commitSymPlusCharacterCycle()
{
    const QString accentedCharacters = hwkbCharLoopsManager.characterLoop(lastSymText[0]);
    inputMethodHost.sendCommitString(QString(accentedCharacters[characterLoopIndex]));
    characterLoopIndex = -1;
    latchModifiers(FnModifierMask | LockMask, 0);
}

bool MHardwareKeyboard::handlePressWithSymModifier(QString &text, quint32 nativeScanCode,
                                                   quint32 &nativeModifiers)
{
    const unsigned char savedLatchedMods(currentLatchedMods);

    if ((characterLoopIndex != -1) && (lastSymText != text)) {
        commitSymPlusCharacterCycle();
    }

    const unsigned char latchedModsDiff(savedLatchedMods ^ currentLatchedMods);
    if (((latchedModsDiff & LockMask) && !shiftsPressed)
        || ((latchedModsDiff & FnModifierMask) && !fnPressed)) {
        text = keycodeToString(nativeScanCode, (fnPressed ? 2 : 0) + (shiftsPressed ? 1 : 0));
        nativeModifiers &= ~((fnPressed ? 0 : FnModifierMask) | (shiftsPressed ? 0 : LockMask));
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
    inputMethodHost.sendPreeditString(accentedCharacters[characterLoopIndex],
                                      MInputMethod::PreeditDefault);
    return true;
}

bool MHardwareKeyboard::handleReleaseWithSymModifier(Qt::Key keyCode)
{
    // Handle possible Sym click...
    if ((lastKeyCode == SymKey) && (keyCode == SymKey)) {
        emit symbolKeyClicked();
        return true;
    } else if (imMode == M::InputMethodModeDirect) { // ...which is all we do for direct mode.
        return false;
    }

    // Maybe stop the ongoing looping.
    if (characterLoopIndex != -1) {
        if (keyCode == SymKey) {
            commitSymPlusCharacterCycle();
        }

        return true;
    }

    // no looping going on, eat sym key release but otherwise let caller handle the event
    return keyCode == SymKey;
}


void MHardwareKeyboard::setAutoCapitalization(bool state)
{
    if (autoCaps != state) {
        // Set auto caps when there is no custom state being set for shift/fn modifier.
        // Also ignore host's attempts to set autocaps to false while Sym+c looping is in
        // progress.
        if (((((currentLockedMods & (FnModifierMask | LockMask)) == 0)
              && ((currentLatchedMods & (FnModifierMask | LockMask)) == 0))
             || autoCaps)
            && !stateTransitionsDisabled && (characterLoopIndex == -1)
            && deadKeyMapper.currentDeadKey().isNull()) {
            if (!(autoCaps && shiftsPressed)) {
                latchModifiers(LockMask, state ? LockMask : 0);
            }
            autoCaps = state;
        }
    }
}


ModifierState MHardwareKeyboard::modifierState(Qt::KeyboardModifier modifier) const
{
    unsigned int latchMask = 0;
    unsigned int lockMask = 0;

    if (modifier == Qt::ShiftModifier) {
        latchMask = LockMask;
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


QChar MHardwareKeyboard::deadKeyState() const
{
    return deadKeyMapper.currentDeadKey();
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
            deadKeyMapper.setLayout(nextLayout, nextVariant);
            LayoutsManager::instance().setXkbMap(nextLayout, nextVariant);
            emit scriptChanged();
        }
    }
}
