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

#include "mxkb_stub.h"
#include "ut_mhardwarekeyboard.h"
#include "hwkbcharloopsmanager_stub.h"
#include "mhardwarekeyboard.h"
#include "testinputcontextconnection.h"
#include "layoutsmanager.h"
#include "utils.h"
#include <MApplication>
#include <QDebug>
#include <QSignalSpy>
#include <QEvent>

#include <X11/X.h>
#undef KeyPress
#undef KeyRelease
#include <X11/XKBlib.h>


namespace
{
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    const Qt::Key FnLevelKey = Qt::Key_AltGr;
    const Qt::Key SymKey = Qt::Key_Multi_key;
    const unsigned int SymModifierMask = Mod4Mask;
    const unsigned int FnModifierMask = Mod5Mask;
    // We need to pass some native X keycode to filterKeyEvent so that it can
    // work correctly.  We don't have to pass correct keycode for every key
    // (besides, correctness depends on the X server), we just need one keycode
    // that makes filterKeyEvent process the event as a non-character key and
    // another that makes filterKeyEvent process the event as a character key.
    // MHardwareKeyboard::keycodeToString() stub will co-operate with those.
    const unsigned int KeycodeCharacter(38); // keycode of "a" under xorg / Xephyr combination
    const unsigned int KeycodeCharacterO(32); // keycode of "o" under xorg / Xephyr combination
    const unsigned int KeycodeCharacterB(56); // keycode of "b" under xorg / Xephyr combination
    const unsigned int KeycodeCharacterPeriod(60); // keycode of "." under xorg / Xephyr combination
    const unsigned int KeycodeNonCharacter(50);  // keycode of left shift under xorg / Xephyr combination

    const QString XkbLayoutSettingName("/meegotouch/inputmethods/hwkeyboard/layout");
    const QString XkbVariantSettingName("/meegotouch/inputmethods/hwkeyboard/variant");
    const QString XkbSecondaryLayoutSettingName("/meegotouch/inputmethods/hwkeyboard/secondarylayout");
    const QString XkbSecondaryVariantSettingName("/meegotouch/inputmethods/hwkeyboard/secondaryvariant");
    const QString XkbModelSettingName("/meegotouch/inputmethods/hwkeyboard/model");
};

Q_DECLARE_METATYPE(Qt::Key)
Q_DECLARE_METATYPE(Qt::KeyboardModifier)
Q_DECLARE_METATYPE(ModifierState)

namespace QTest
{
template <>
char *toString(const ModifierState &state)
{
    QString string;
    QDebug debug(&string);

    switch (state) {
    case ModifierClearState:
        debug << "clear";
        break;
    case ModifierLatchedState:
        debug << "latched";
        break;
    case ModifierLockedState:
        debug << "locked";
        break;
    }

    return qstrdup(qPrintable(string));
}
}


// Stubbing..................................................................

QString MHardwareKeyboard::keycodeToString(unsigned int keycode, unsigned int shiftLevel) const
{
    if (keycode == KeycodeNonCharacter) {
        return QString();
    } else if ((keycode == KeycodeCharacterO) && (shiftLevel == 0)) {
        return QString("o");
    } else if ((keycode == KeycodeCharacterB) && (shiftLevel == 0)) {
        return QString("b");
    } else if (keycode == KeycodeCharacterPeriod) {
        return QString(".");
    } else {
        return QString("a");    // just an arbitrary character
    }
}


// Test init.................................................................

void Ut_MHardwareKeyboard::initTestCase()
{
    disableQtPlugins();

    static char *argv[3] = {(char *)"ut_mhardwarekeyboard", (char *)"-software", (char *) "-local-theme"};
    static int argc = 3;
    app = new MApplication(argc, argv);

    qRegisterMetaType<ModifierState>("ModifierState");
}

void Ut_MHardwareKeyboard::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ut_MHardwareKeyboard::init()
{
    inputContextConnection = new TestInputContextConnection;
    m_hkb = new MHardwareKeyboard(*inputContextConnection, 0);
    m_hkb->reset();
    m_hkb->enable();
    m_hkb->setKeyboardType(M::FreeTextContentType);
    m_hkb->setInputMethodMode(M::InputMethodModeNormal);
}

void Ut_MHardwareKeyboard::cleanup()
{
    delete m_hkb;
    delete inputContextConnection;
}


// Utilities.................................................................

void Ut_MHardwareKeyboard::setState(const int state) const
{
    switch (state) {
    case 0:
        break;
    case 1:
        filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);
        filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
        break;
    case 2:
        filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);
        filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
        filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
        filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
        break;
    case 3:
        m_hkb->setAutoCapitalization(true);
        break;
    case 4:
        filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, 0);
        filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
        break;
    case 5:
        filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, 0);
        filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
        filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
        filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
        break;
    }
}

bool Ut_MHardwareKeyboard::filterKeyRelease(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                                            const QString &text,
                                            quint32 nativeScanCode, quint32 nativeModifiers) const
{
    return m_hkb->filterKeyEvent(QEvent::KeyRelease, keyCode, modifiers, text, false, 1, nativeScanCode,
                                 nativeModifiers);
}

bool Ut_MHardwareKeyboard::filterKeyPress(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                                          const QString &text,
                                          quint32 nativeScanCode, quint32 nativeModifiers) const
{
    return m_hkb->filterKeyEvent(QEvent::KeyPress, keyCode, modifiers, text, false, 1, nativeScanCode,
                                 nativeModifiers);
}

bool Ut_MHardwareKeyboard::checkLatchedState(const unsigned int mask, const unsigned int value) const
{
    XkbStateRec xkbState;
    XkbGetState(QX11Info::display(), XkbUseCoreKbd, &xkbState); // TODO: XkbUseCoreKbd
    qDebug() << "Latched/Xkb:" << (xkbState.latched_mods & mask)
             << "latched/hwkbd:" << (m_hkb->currentLatchedMods & mask);
    return ((xkbState.latched_mods & mask) == value)
        && ((m_hkb->currentLatchedMods & mask) == value);
}

bool Ut_MHardwareKeyboard::checkLockedState(const unsigned int mask, const unsigned int value) const
{
    XkbStateRec xkbState;
    XkbGetState(QX11Info::display(), XkbUseCoreKbd, &xkbState); // TODO: XkbUseCoreKbd
    qDebug() << "Locked/Xkb:" << (xkbState.locked_mods & mask)
             << "locked/hwkbd:" << (m_hkb->currentLockedMods & mask);
    return ((xkbState.locked_mods & mask) == value)
        && ((m_hkb->currentLockedMods & mask) == value);
}


// Tests.....................................................................

void Ut_MHardwareKeyboard::testBasicModifierCycles_data()
{
    QTest::addColumn<Qt::Key>("key");
    QTest::addColumn<unsigned int>("latchMask");
    QTest::addColumn<unsigned int>("lockMask");
    QTest::newRow("Shift") << Qt::Key_Shift
                           << static_cast<unsigned int>(LockMask)
                           << static_cast<unsigned int>(LockMask);
    QTest::newRow("Fn") << FnLevelKey
                        << static_cast<unsigned int>(FnModifierMask)
                        << static_cast<unsigned int>(FnModifierMask);
}

void Ut_MHardwareKeyboard::testBasicModifierCycles()
{
    QFETCH(Qt::Key, key);
    QFETCH(unsigned int, latchMask);
    QFETCH(unsigned int, lockMask);

    QSignalSpy modifierSpy(m_hkb, SIGNAL(modifiersStateChanged()));
    QVERIFY(modifierSpy.isValid());
    QSignalSpy shiftSpy(m_hkb, SIGNAL(shiftStateChanged()));
    QVERIFY(shiftSpy.isValid());

    // Latch
    QVERIFY(filterKeyPress(key, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(filterKeyRelease(key, Qt::NoModifier, "", KeycodeNonCharacter, latchMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, latchMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), key == Qt::Key_Shift ? 1 : 0);
    modifierSpy.clear();
    shiftSpy.clear();

    // Autocaps ignored in latched state?
    QVERIFY(!m_hkb->autoCaps);
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, latchMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));

    // Lock
    QVERIFY(filterKeyPress(key, Qt::NoModifier, "", KeycodeNonCharacter, latchMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, latchMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(filterKeyRelease(key, Qt::NoModifier, "", KeycodeNonCharacter, latchMask));
    QVERIFY(checkLatchedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, lockMask));
    QCOMPARE(modifierSpy.count(), 2);
    if (key == Qt::Key_Shift) {
        QVERIFY(shiftSpy.count() >= 1);
    } else {
        QCOMPARE(shiftSpy.count(), 0);
    }
    modifierSpy.clear();
    shiftSpy.clear();

    // Autocaps ignored in locked state?
    QVERIFY(!m_hkb->autoCaps);
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(checkLatchedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, lockMask));

    // Unlock
    QVERIFY(filterKeyPress(key, Qt::NoModifier, "", KeycodeNonCharacter, lockMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, lockMask));
    QVERIFY(filterKeyRelease(key, Qt::NoModifier, "", KeycodeNonCharacter, lockMask | latchMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), key == Qt::Key_Shift ? 1 : 0);
    modifierSpy.clear();
    shiftSpy.clear();

    // Release without press doesn't change state
    QVERIFY(filterKeyPress(key, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "A", KeycodeNonCharacter, latchMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "A", KeycodeNonCharacter, latchMask));
    QVERIFY(filterKeyRelease(key, Qt::NoModifier, "", KeycodeNonCharacter, latchMask));
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
}


void Ut_MHardwareKeyboard::testAutoCaps()
{
    QSignalSpy modifierSpy(m_hkb, SIGNAL(modifiersStateChanged()));
    QVERIFY(modifierSpy.isValid());
    QSignalSpy shiftSpy(m_hkb, SIGNAL(shiftStateChanged()));
    QVERIFY(shiftSpy.isValid());

    // Autocaps on
    QVERIFY(!m_hkb->autoCaps);
    m_hkb->setAutoCapitalization(true);
    QVERIFY(m_hkb->autoCaps);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, LockMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), 1);

    // Autocaps off
    m_hkb->setAutoCapitalization(false);
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    modifierSpy.clear();
    shiftSpy.clear();

    // Autocaps on again
    m_hkb->setAutoCapitalization(true);
    QVERIFY(m_hkb->autoCaps);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, LockMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), 1);
    modifierSpy.clear();
    shiftSpy.clear();

    // Shift click turns autocaps off and unlatches shift (instead of normal latched -> locked)
    QVERIFY(filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, LockMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), 1);

    // Afterwards normal clear -> latched -> locked cycle works
    QVERIFY(filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, LockMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, LockMask));
    QVERIFY(filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, LockMask));
    QVERIFY(filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, LockMask | ShiftMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));

    modifierSpy.clear();
    shiftSpy.clear();

    // Autocaps is ignored in [phone] number keyboard state
    m_hkb->setKeyboardType(M::PhoneNumberContentType);
    int countBeforeAutoCaps = modifierSpy.count();
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));
    QCOMPARE(modifierSpy.count(), countBeforeAutoCaps);
    QCOMPARE(shiftSpy.count(), 0);
    modifierSpy.clear();

    m_hkb->enable();
    m_hkb->setKeyboardType(M::NumberContentType);
    countBeforeAutoCaps = modifierSpy.count();
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));
    QCOMPARE(modifierSpy.count(), countBeforeAutoCaps);
    QCOMPARE(shiftSpy.count(), 0);
    m_hkb->enable();
    m_hkb->setKeyboardType(M::FreeTextContentType);
    modifierSpy.clear();

    // Autocaps is ignored when Sym+ccc... is in progress
    QVERIFY(filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));

    // Works again now that we stopped Sym+ccc...
    m_hkb->setAutoCapitalization(true);
    QVERIFY(m_hkb->autoCaps);
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, LockMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), 1);

    // Reset doesn't reset autocaps
    m_hkb->reset();
    QVERIFY(m_hkb->autoCaps);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, LockMask));
}


void Ut_MHardwareKeyboard::testStateReset_data()
{
    QTest::addColumn<int>("state");
    QTest::newRow("Clear") << 0;
    QTest::newRow("Shift latched") << 1;
    QTest::newRow("Shift locked") << 2;
    QTest::newRow("Autocaps") << 3;
    QTest::newRow("Fn latched") << 4;
    QTest::newRow("Fn locked") << 5;
}

void Ut_MHardwareKeyboard::testStateReset()
{
    QFETCH(int, state);

    setState(state);

    QSignalSpy modifierSpy(m_hkb, SIGNAL(modifiersStateChanged()));
    QVERIFY(modifierSpy.isValid());

    // State is cleared on focus in...
    m_hkb->enable();
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(!m_hkb->autoCaps);
    // ...and we always get notifications, even in clear state
    QCOMPARE(modifierSpy.count(), 1);

    // Modifiers are also unlatched/unlocked on focus out.  We don't care about
    // notifications.
    setState(state);
    m_hkb->disable();
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(!m_hkb->autoCaps);
}


void Ut_MHardwareKeyboard::testModifierInNonTextContentType_data()
{
    QTest::addColumn<M::TextContentType>("contentType");
    QTest::newRow("Number") << M::NumberContentType;
    QTest::newRow("PhoneNumber") << M::PhoneNumberContentType;
}

void Ut_MHardwareKeyboard::testModifierInNonTextContentType()
{
    QFETCH(M::TextContentType, contentType);

    m_hkb->setKeyboardType(contentType);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));

    // Shift won't change the state
    QVERIFY(filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask | ShiftMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));

    // Neither does Fn
    QVERIFY(filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));

    // Neither does Shift+Shift
    QVERIFY(filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask | ShiftMask));
    QVERIFY(filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask | ShiftMask));
    QVERIFY(filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask | ShiftMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));
}


void Ut_MHardwareKeyboard::testShiftShiftCapsLock_data()
{
    QTest::addColumn<int>("state");
    QTest::newRow("Clear") << 0;
    QTest::newRow("Shift latched") << 1;
    QTest::newRow("Shift locked") << 2;
    QTest::newRow("Autocaps") << 3;
    QTest::newRow("Fn latched") << 4;
    QTest::newRow("Fn locked") << 5;
}

void Ut_MHardwareKeyboard::testShiftShiftCapsLock()
{
    QFETCH(int, state);

    setState(state);

    // Shift+Shift -> lock
    QVERIFY(filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, LockMask));

    // Until we have released both shift keys, shift releases don't change state
    for (int i = 0; i < 2; ++i) {
        QVERIFY(filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, LockMask | ShiftMask));
        QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
        QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, LockMask));
    }

    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, LockMask);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
}


void Ut_MHardwareKeyboard::testOtherModifier_data()
{
    QTest::addColumn<int>("state");
    QTest::addColumn<Qt::Key>("otherKey");
    QTest::addColumn<unsigned int>("latchedMask");
    QTest::newRow("Shift latched") << 1 << FnLevelKey << static_cast<unsigned int>(FnModifierMask);
    QTest::newRow("Shift locked") << 2 << FnLevelKey << static_cast<unsigned int>(FnModifierMask);
    QTest::newRow("Autocaps") << 3 << FnLevelKey << static_cast<unsigned int>(FnModifierMask);
    QTest::newRow("Fn latched") << 4 << Qt::Key_Shift << static_cast<unsigned int>(LockMask);
    QTest::newRow("Fn locked") << 5 << Qt::Key_Shift << static_cast<unsigned int>(LockMask);
}

void Ut_MHardwareKeyboard::testOtherModifier()
{
    QFETCH(int, state);
    QFETCH(Qt::Key, otherKey);
    QFETCH(unsigned int, latchedMask);

    // If, in the given state,...
    setState(state);

    // ...we click other (that is, other that the one that got us into this state ;) modifier key...
    // (note: native modifier mask and modifiers are not correct but it doesn't matter)
    filterKeyPress(otherKey, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    QVERIFY(filterKeyRelease(otherKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));

    // ...we should end up to a latched state for that modifier.
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, latchedMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(!m_hkb->autoCaps);
}


void Ut_MHardwareKeyboard::testSymClick()
{
    QSignalSpy symSpy(m_hkb, SIGNAL(symbolKeyClicked()));
    QVERIFY(symSpy.isValid());

    // Press+release gives us signal
    QVERIFY(filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));
    QCOMPARE(symSpy.count(), 1);
    symSpy.clear();

    // Sym press + something + sym release doesn't
    QVERIFY(filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));
    QCOMPARE(symSpy.count(), 0);
}


void Ut_MHardwareKeyboard::testSymPlusCharacterBasic_data()
{
    QTest::addColumn<int>("iterations");
    QTest::addColumn<QChar>("result");
    QTest::newRow("1") << 1 << QChar(0x00E4);
    QTest::newRow("2") << 2 << QChar(0x00E0);
    QTest::newRow("3") << 3 << QChar(0x00E2);
    QTest::newRow("4") << 4 << QChar(0x00E1);
    QTest::newRow("5") << 5 << QChar(0x00E3);
    QTest::newRow("6") << 6 << QChar(0x00E5);
    QTest::newRow("7") << 7 << QChar(0x00E4); // Wrap around
}

void Ut_MHardwareKeyboard::testSymPlusCharacterBasic()
{
    QFETCH(int, iterations);
    QFETCH(QChar, result);

    QSignalSpy symSpy(m_hkb, SIGNAL(symbolKeyClicked()));
    QVERIFY(symSpy.isValid());

    // Basic case: SymP(ress)+a{1,n}+SymR(elease)
    QVERIFY(filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    for (int i = 0; i < iterations; ++i) {
        inputContextConnection->sendPreeditString("", PreeditNoCandidates);
        QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
        QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
        QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
        QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
        QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    }
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(result));

    QVERIFY(filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));
    QCOMPARE(inputContextConnection->keyEventsSent(), static_cast<unsigned int >(0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 1);
    QCOMPARE(inputContextConnection->lastCommitString(), QString(result));

    QCOMPARE(inputContextConnection->keyEventsSent(), static_cast<unsigned int >(0));
    QCOMPARE(symSpy.count(), 0);
}


void Ut_MHardwareKeyboard::testSymPlusCharSwitchs()
{
    QSignalSpy symSpy(m_hkb, SIGNAL(symbolKeyClicked()));
    QVERIFY(symSpy.isValid());

    // SymP+a+a+o+b+SymR commits result of Sym+a+a on 'o' press...
    QVERIFY(filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(filterKeyPress(Qt::Key_O, Qt::NoModifier, "o", KeycodeCharacter, SymModifierMask));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastCommitString(), QString(QChar(0x00E4)));
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(QChar(0x00F6)));
    inputContextConnection->sendCommitString("");
    inputContextConnection->sendPreeditString("", PreeditDefault);
    QVERIFY(filterKeyRelease(Qt::Key_O, Qt::NoModifier, "o", KeycodeCharacter, SymModifierMask));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 0);

    // ...and result of Sym+o is committed on b press.  Since b has no loops, it's not
    // handled by the sym+foo logic, which we can tell by observing that long press logic
    // is activated for it.
    QVERIFY(filterKeyPress(Qt::Key_B, Qt::NoModifier, "b", KeycodeCharacter, SymModifierMask));
    QCOMPARE(inputContextConnection->lastCommitString(), QString(QChar(0x00F6)));
    QCOMPARE(inputContextConnection->lastPreeditString(), QString("b"));
    QVERIFY(filterKeyRelease(Qt::Key_B, Qt::NoModifier, "b", KeycodeCharacter, SymModifierMask));
    QCOMPARE(inputContextConnection->lastCommitString(), QString("b"));

    QVERIFY(filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));

    // Sym with latched shift, case 1: another loop after loop in latched state
    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, LockMask));
    QVERIFY(filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, LockMask));

    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "A", KeycodeCharacter, SymModifierMask | LockMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "A", KeycodeCharacter, SymModifierMask | LockMask));
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(QChar(0x00C4)));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, LockMask));
    // The following press must unlatch shift and be handled as if "o" event was received...
    QVERIFY(filterKeyPress(Qt::Key_O, Qt::NoModifier, "O", KeycodeCharacterO, SymModifierMask | LockMask));
    QVERIFY(checkLatchedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(inputContextConnection->lastCommitString(), QString(QChar(0x00C4)));
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(QChar(0x00F6)));
    QVERIFY(filterKeyRelease(Qt::Key_O, Qt::NoModifier, "o", KeycodeCharacterO, SymModifierMask));
    // ...which means that this must continue started cycle, not start a new one
    QVERIFY(filterKeyPress(Qt::Key_O, Qt::NoModifier, "o", KeycodeCharacterO, SymModifierMask));
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(QChar(0x00F2)));
    QVERIFY(filterKeyRelease(Qt::Key_O, Qt::NoModifier, "o", KeycodeCharacterO, SymModifierMask));

    QVERIFY(filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));

    // Sym with latched shift, case 2: "no loops" key after loop in latched state
    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, LockMask));
    QVERIFY(filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, LockMask));

    // Start looping, take two steps
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "A", KeycodeCharacter, SymModifierMask | LockMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "A", KeycodeCharacter, SymModifierMask | LockMask));
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "A", KeycodeCharacter, SymModifierMask | LockMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "A", KeycodeCharacter, SymModifierMask | LockMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, LockMask));

    // Key for which there is no loop defined
    QVERIFY(filterKeyPress(Qt::Key_O, Qt::NoModifier, "B", KeycodeCharacterB, SymModifierMask | LockMask));
    QCOMPARE(inputContextConnection->lastCommitString(), QString(QChar(0x00C0)));
    QCOMPARE(inputContextConnection->lastPreeditString(), QString("b"));
    QCOMPARE(m_hkb->longPressKey, KeycodeCharacterB);
    QCOMPARE(m_hkb->longPressModifiers, SymModifierMask);
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, 0));
    QVERIFY(filterKeyRelease(Qt::Key_O, Qt::NoModifier, "b", KeycodeCharacterO, SymModifierMask));

    QCOMPARE(symSpy.count(), 0);
}


void Ut_MHardwareKeyboard::testDelete_data()
{
    QTest::addColumn<int>("state");
    QTest::newRow("Clear") << 0;
    QTest::newRow("Shift latched") << 1;
    QTest::newRow("Shift locked") << 2;
    QTest::newRow("Fn latched") << 4;
    QTest::newRow("Fn locked") << 5;
}

void Ut_MHardwareKeyboard::testDelete()
{
    QFETCH(int, state);

    setState(state);

    // Native and Qt modifiers (except for Qt::ShiftModifier) in all of the
    // following filterKey* calls don't match the reality.

    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);

    QVERIFY(filterKeyPress(Qt::Key_Delete, Qt::ShiftModifier, "\177", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->keyEventsSent(), static_cast<unsigned int>(1));
    QCOMPARE(inputContextConnection->lastKeyEvent().type(), QEvent::KeyPress);
    QCOMPARE(inputContextConnection->lastKeyEvent().key(), static_cast<int>(Qt::Key_Delete));
    QCOMPARE(inputContextConnection->lastKeyEvent().modifiers(), Qt::NoModifier);
    QCOMPARE(inputContextConnection->lastKeyEvent().text(), QString("\177"));

    QVERIFY(filterKeyRelease(Qt::Key_Delete, Qt::ShiftModifier, "\177", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->keyEventsSent(), static_cast<unsigned int>(2));
    QCOMPARE(inputContextConnection->lastKeyEvent().type(), QEvent::KeyRelease);
    QCOMPARE(inputContextConnection->lastKeyEvent().key(), static_cast<int>(Qt::Key_Delete));
    QCOMPARE(inputContextConnection->lastKeyEvent().modifiers(), Qt::NoModifier);
    QCOMPARE(inputContextConnection->lastKeyEvent().text(), QString("\177"));

    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);
}

void Ut_MHardwareKeyboard::testDirectInputMode()
{
    m_hkb->setInputMethodMode(M::InputMethodModeDirect);
    QCOMPARE(m_hkb->inputMethodMode(), M::InputMethodModeDirect);

    QSignalSpy symSpy(m_hkb, SIGNAL(symbolKeyClicked()));
    QVERIFY(symSpy.isValid());

    // Press+release gives us signal
    QVERIFY(filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));
    QCOMPARE(symSpy.count(), 1);
    symSpy.clear();

    // Anyother key will be ignored
    QVERIFY(!filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, 0));
    QVERIFY(!filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 0);

    // shift and fn will be ignored
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(!filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));

    QVERIFY(!filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(!filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(checkLatchedState(LockMask | FnModifierMask, 0));

    QCOMPARE(symSpy.count(), 0);
}

void Ut_MHardwareKeyboard::testSwitchLayout()
{
    const QString primaryLayout("ru");
    const QString primaryVariant("cyrillic");
    const QString secondaryLayout("ru");
    const QString secondaryVariant("latin");

    MGConfItem layoutConfig(XkbLayoutSettingName);
    layoutConfig.set(QVariant(primaryLayout));
    MGConfItem variantConfig(XkbVariantSettingName);
    variantConfig.set(QVariant(primaryVariant));

    MGConfItem secondaryLayoutConfig(XkbSecondaryLayoutSettingName);
    secondaryLayoutConfig.set(QVariant(secondaryLayout));
    MGConfItem secondaryVariantConfig(XkbSecondaryVariantSettingName);
    secondaryVariantConfig.set(QVariant(secondaryVariant));

    LayoutsManager::createInstance(0);
    gSetXkbMapCallCount = 0;
    int switchCount = 0;
    QSignalSpy scriptChangedSpy(m_hkb, SIGNAL(scriptChanged()));

    // shift + fn press together switch layout
    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    // won't switch for press
    QCOMPARE(gSetXkbMapCallCount, switchCount);
    // won't change shift fn level
    QVERIFY(checkLatchedState(ShiftMask | LockMask | FnModifierMask, 0));

    QCOMPARE(scriptChangedSpy.count(), 0);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask|FnModifierMask);
    // only switch when one of fn/shift release
    QCOMPARE(gSetXkbMapCallCount, ++switchCount);
    QCOMPARE(scriptChangedSpy.count(), 1);
    QCOMPARE(gLayout, secondaryLayout);
    QCOMPARE(gVariant, secondaryVariant);
    filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
    // won't change shift fn level
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, 0));

    // no dependency on shift/fn press/release order
    scriptChangedSpy.clear();
    filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask|FnModifierMask);
    filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
    QCOMPARE(gSetXkbMapCallCount, ++switchCount);
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, 0));
    QCOMPARE(gLayout, primaryLayout);
    QCOMPARE(gVariant, primaryVariant);
    QCOMPARE(scriptChangedSpy.count(), 1);

    scriptChangedSpy.clear();
    filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
    filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask|FnModifierMask);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    QCOMPARE(gSetXkbMapCallCount, ++switchCount);
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, 0));
    QCOMPARE(gLayout, secondaryLayout);
    QCOMPARE(gVariant, secondaryVariant);
    QCOMPARE(scriptChangedSpy.count(), 1);

    scriptChangedSpy.clear();
    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask|FnModifierMask);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    QCOMPARE(gSetXkbMapCallCount, ++switchCount);
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, 0));
    QCOMPARE(gLayout, primaryLayout);
    QCOMPARE(gVariant, primaryVariant);
    QCOMPARE(scriptChangedSpy.count(), 1);

    scriptChangedSpy.clear();
    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, FnModifierMask|ShiftMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, FnModifierMask|ShiftMask));
    filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask|FnModifierMask);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    // if some character key is pressed when shift+fn is held,
    // won't change layout
    QCOMPARE(gSetXkbMapCallCount, switchCount);
    QCOMPARE(gLayout, primaryLayout);
    QCOMPARE(gVariant, primaryVariant);
    QCOMPARE(scriptChangedSpy.count(), 0);

    scriptChangedSpy.clear();
    inputContextConnection->sendPreeditString("", PreeditNoCandidates);
    inputContextConnection->sendCommitString("");
    // ctrl + space press together also switch layout
    filterKeyPress(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyPress(Qt::Key_Space, Qt::ControlModifier, " ", KeycodeNonCharacter, ControlMask);
    // switching happens when receive space press
    QCOMPARE(gSetXkbMapCallCount, ++switchCount);
    QCOMPARE(scriptChangedSpy.count(), 1);
    QCOMPARE(gLayout, secondaryLayout);
    QCOMPARE(gVariant, secondaryVariant);
    filterKeyRelease(Qt::Key_Space, Qt::ControlModifier, "", KeycodeNonCharacter, ControlMask);
    filterKeyRelease(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, ControlMask);
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, 0));
    // won't switch again for key release
    QCOMPARE(gSetXkbMapCallCount, switchCount);
    QCOMPARE(scriptChangedSpy.count(), 1);

    //space key press and release when control is holding won't insert space
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);

    scriptChangedSpy.clear();
    filterKeyPress(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyPress(Qt::Key_Space, Qt::ControlModifier, " ", KeycodeNonCharacter, ControlMask);
    QCOMPARE(gSetXkbMapCallCount, ++switchCount);
    QCOMPARE(scriptChangedSpy.count(), 1);
    QCOMPARE(gLayout, primaryLayout);
    QCOMPARE(gVariant, primaryVariant);
    filterKeyRelease(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, ControlMask);
    filterKeyRelease(Qt::Key_Space, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, 0));
    // won't switch again for key release
    QCOMPARE(gSetXkbMapCallCount, switchCount);
    QCOMPARE(scriptChangedSpy.count(), 1);

    //space key press and release when control is holding won't insert space
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);

    scriptChangedSpy.clear();
    // shift + ctrl + space won't switch script
    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyPress(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    filterKeyPress(Qt::Key_Space, Qt::ControlModifier, " ", KeycodeNonCharacter, ShiftMask|ControlMask);
    // won't switch
    QCOMPARE(gSetXkbMapCallCount, switchCount);
    QCOMPARE(scriptChangedSpy.count(), 0);
    QCOMPARE(gLayout, primaryLayout);
    QCOMPARE(gVariant, primaryVariant);
    filterKeyRelease(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask|ControlMask);
    filterKeyRelease(Qt::Key_Space, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, 0));
    // won't switch
    QCOMPARE(gSetXkbMapCallCount, switchCount);
    QCOMPARE(scriptChangedSpy.count(), 0);

    // fn + ctrl + space won't switch script
    filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    filterKeyPress(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
    filterKeyPress(Qt::Key_Space, Qt::ControlModifier, " ", KeycodeNonCharacter, FnModifierMask|ControlMask);
    // won't switch
    QCOMPARE(gSetXkbMapCallCount, switchCount);
    QCOMPARE(scriptChangedSpy.count(), 0);
    QCOMPARE(gLayout, primaryLayout);
    QCOMPARE(gVariant, primaryVariant);
    filterKeyRelease(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask|ControlMask);
    filterKeyRelease(Qt::Key_Space, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
    filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask);
    QVERIFY(checkLatchedState(LockMask | ShiftMask | FnModifierMask, 0));
    // won't switch
    QCOMPARE(gSetXkbMapCallCount, switchCount);
    QCOMPARE(scriptChangedSpy.count(), 0);

    LayoutsManager::destroyInstance();
}

void Ut_MHardwareKeyboard::testControlModifier()
{
    // We must not eat events with control modifier
    filterKeyPress(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    QVERIFY(!filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, ControlMask));
    QVERIFY(!filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, ControlMask));
    filterKeyRelease(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, ControlMask);

    // We also pass character release if it was pressed with control modifier.  This is a
    // rather arbitrary decision and actually the main thing is that we don't try to
    // commit anything (which would kill the selection).
    inputContextConnection->sendCommitString("foo");
    filterKeyPress(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, 0);
    QVERIFY(!filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, ControlMask));
    filterKeyRelease(Qt::Key_Control, Qt::NoModifier, "", KeycodeNonCharacter, ControlMask);
    QVERIFY(!filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString(), QString("foo"));
}


void Ut_MHardwareKeyboard::testCorrectToAcceptedCharacter()
{
    m_hkb->setKeyboardType(M::NumberContentType);
    QVERIFY(filterKeyPress(Qt::Key_Colon, Qt::GroupSwitchModifier, ":", KeycodeCharacterPeriod, FnModifierMask));
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString("."));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QVERIFY(filterKeyRelease(Qt::Key_Colon, Qt::GroupSwitchModifier, ":", KeycodeCharacterPeriod, FnModifierMask));
    QCOMPARE(inputContextConnection->lastCommitString(), QString("."));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 1);
}


void Ut_MHardwareKeyboard::testKeyInsertionOnReleaseAfterReset()
{
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, 0));
    m_hkb->reset();
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
}


void Ut_MHardwareKeyboard::testDeadKeys()
{
    QSignalSpy deadKeyStateSpy(m_hkb, SIGNAL(deadKeyStateChanged(const QChar &)));

    MGConfItem layoutConfig(XkbLayoutSettingName);
    layoutConfig.set("fr");
    MGConfItem variantConfig(XkbVariantSettingName);
    variantConfig.set("");

    // The basic case, ^ + a => \^a

    QVERIFY(filterKeyPress(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString("^"));
    QCOMPARE(deadKeyStateSpy.count(), 1);
    QCOMPARE(deadKeyStateSpy.at(0).at(0).value<QChar>(), QChar('^'));
    deadKeyStateSpy.clear();
    QCOMPARE(m_hkb->deadKeyState(), QChar('^'));
    QVERIFY(filterKeyRelease(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString("^"));

    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(QChar(0xe2)));
    QCOMPARE(deadKeyStateSpy.count(), 1);
    QCOMPARE(deadKeyStateSpy.at(0).at(0).value<QChar>(), QChar());
    deadKeyStateSpy.clear();
    QCOMPARE(m_hkb->deadKeyState(), QChar());

    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 1);
    QCOMPARE(inputContextConnection->lastCommitString(), QString(QChar(0xe2)));

    inputContextConnection->sendCommitString("");
    inputContextConnection->sendPreeditString("", PreeditDefault);

    // Switch dead key, ^ + \" + a => \"a

    QVERIFY(filterKeyPress(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));
    QVERIFY(filterKeyRelease(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString("^"));

    QVERIFY(filterKeyPress(Qt::Key_Dead_Diaeresis, Qt::NoModifier, QString(QChar(0xa8)), KeycodeCharacter, 0));
    QVERIFY(filterKeyRelease(Qt::Key_Dead_Diaeresis, Qt::NoModifier, QString(QChar(0xa8)), KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(QChar(0xa8)));
    QCOMPARE(deadKeyStateSpy.count(), 2);
    QCOMPARE(deadKeyStateSpy.at(1).at(0).value<QChar>(), QChar(0xa8));
    deadKeyStateSpy.clear();
    QCOMPARE(m_hkb->deadKeyState(), QChar(0xa8));

    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(QChar(0xe4)));

    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 1);
    QCOMPARE(inputContextConnection->lastCommitString(), QString(QChar(0xe4)));

    inputContextConnection->sendCommitString("");
    inputContextConnection->sendPreeditString("", PreeditDefault);

    // Dead key with space

    QVERIFY(filterKeyPress(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));
    QVERIFY(filterKeyRelease(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));

    QVERIFY(filterKeyPress(Qt::Key_Space, Qt::NoModifier, " ", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString("^"));

    QVERIFY(filterKeyRelease(Qt::Key_Space, Qt::NoModifier, " ", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 1);
    QCOMPARE(inputContextConnection->lastCommitString(), QString("^"));
    QCOMPARE(m_hkb->deadKeyState(), QChar());

    inputContextConnection->sendCommitString("");
    inputContextConnection->sendPreeditString("", PreeditDefault);

    // Dead key with a key it cannot be combined with

    QVERIFY(filterKeyPress(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));
    QVERIFY(filterKeyRelease(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));

    QVERIFY(filterKeyPress(Qt::Key_D, Qt::NoModifier, "d", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString("d"));

    QVERIFY(filterKeyRelease(Qt::Key_Space, Qt::NoModifier, "d", KeycodeCharacter, 0));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 1);
    QCOMPARE(inputContextConnection->lastCommitString(), QString("d"));
    QCOMPARE(m_hkb->deadKeyState(), QChar());

    inputContextConnection->sendCommitString("");
    inputContextConnection->sendPreeditString("", PreeditDefault);

    // Reset resets dead key mapper state

    QVERIFY(filterKeyPress(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));
    QVERIFY(filterKeyRelease(Qt::Key_Dead_Circumflex, Qt::NoModifier, "^", KeycodeCharacter, 0));
    QCOMPARE(m_hkb->deadKeyState(), QChar('^'));

    m_hkb->reset();

    QCOMPARE(m_hkb->deadKeyState(), QChar());
}


QTEST_APPLESS_MAIN(Ut_MHardwareKeyboard);
