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



#include "ut_mhardwarekeyboard.h"
#include "hwkbcharloopsmanager_stub.h"
#include "mhardwarekeyboard.h"
#include "testinputcontextconnection.h"
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
    const unsigned int KeycodeNonCharacter(50);  // keycode of left shift under xorg / Xephyr combination
};

Q_DECLARE_METATYPE(Qt::Key)
Q_DECLARE_METATYPE(Qt::KeyboardModifier)
Q_DECLARE_METATYPE(ModifierState)
Q_DECLARE_METATYPE(M::TextContentType)

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

QString MHardwareKeyboard::keycodeToString(unsigned int keycode, unsigned int /* shiftLevel */) const
{
    if (keycode == KeycodeNonCharacter) {
        return QString();
    } else {
        return QString("'");    // just an arbitrary character
    }
}


// Test init.................................................................

void Ut_MHardwareKeyboard::initTestCase()
{
    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);

    static char *argv[2] = {(char *)"ut_mhardwarekeyboard", (char *)"-software"};
    static int argc = 2;
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
    m_hkb->setKeyboardType(M::FreeTextContentType);
}

void Ut_MHardwareKeyboard::cleanup()
{
    delete m_hkb;
    delete inputContextConnection;
}


// Tests.....................................................................

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


void Ut_MHardwareKeyboard::testBasicModifierCycles_data()
{
    QTest::addColumn<Qt::Key>("key");
    QTest::addColumn<Qt::KeyboardModifier>("modifier");
    QTest::addColumn<unsigned int>("latchMask");
    QTest::addColumn<unsigned int>("lockMask");
    QTest::newRow("Shift") << Qt::Key_Shift << Qt::ShiftModifier
                           << static_cast<unsigned int>(ShiftMask)
                           << static_cast<unsigned int>(LockMask);
    QTest::newRow("Fn") << FnLevelKey << FnLevelModifier
                        << static_cast<unsigned int>(FnModifierMask)
                        << static_cast<unsigned int>(FnModifierMask);
}

void Ut_MHardwareKeyboard::testBasicModifierCycles()
{
    QFETCH(Qt::Key, key);
    QFETCH(Qt::KeyboardModifier, modifier);
    QFETCH(unsigned int, latchMask);
    QFETCH(unsigned int, lockMask);

    QSignalSpy modifierSpy(m_hkb, SIGNAL(modifierStateChanged(Qt::KeyboardModifier, ModifierState)));
    QVERIFY(modifierSpy.isValid());
    QSignalSpy shiftSpy(m_hkb, SIGNAL(shiftStateChanged()));
    QVERIFY(shiftSpy.isValid());

    // Latch
    QVERIFY(!filterKeyPress(key, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(!filterKeyRelease(key, Qt::NoModifier, "", KeycodeNonCharacter, latchMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, latchMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), key == Qt::Key_Shift ? 1 : 0);
    QCOMPARE(modifierSpy.at(0).at(0).value<Qt::KeyboardModifier>(), modifier);
    QCOMPARE(modifierSpy.at(0).at(1).value<ModifierState>(), ModifierLatchedState);
    modifierSpy.clear();
    shiftSpy.clear();

    // Autocaps ignored in latched state?
    QVERIFY(!m_hkb->autoCaps);
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, latchMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));

    // Lock
    QVERIFY(!filterKeyPress(key, Qt::NoModifier, "", KeycodeNonCharacter, latchMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, latchMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(!filterKeyRelease(key, Qt::NoModifier, "", KeycodeNonCharacter, latchMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, lockMask));
    QCOMPARE(modifierSpy.count(), 2);
    if (key == Qt::Key_Shift) {
        QVERIFY(shiftSpy.count() >= 1);
    } else {
        QCOMPARE(shiftSpy.count(), 0);
    }
    QCOMPARE(modifierSpy.at(1).at(0).value<Qt::KeyboardModifier>(), modifier);
    QCOMPARE(modifierSpy.at(1).at(1).value<ModifierState>(), ModifierLockedState);
    modifierSpy.clear();
    shiftSpy.clear();

    // Autocaps ignored in locked state?
    QVERIFY(!m_hkb->autoCaps);
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, lockMask));

    // Unlock
    QVERIFY(!filterKeyPress(key, Qt::NoModifier, "", KeycodeNonCharacter, lockMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, lockMask));
    QVERIFY(!filterKeyRelease(key, Qt::NoModifier, "", KeycodeNonCharacter, lockMask | latchMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), key == Qt::Key_Shift ? 1 : 0);
    QCOMPARE(modifierSpy.at(0).at(0).value<Qt::KeyboardModifier>(), modifier);
    QCOMPARE(modifierSpy.at(0).at(1).value<ModifierState>(), ModifierClearState);
    modifierSpy.clear();
    shiftSpy.clear();

    // Release without press doesn't change state
    QVERIFY(!filterKeyPress(key, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "A", KeycodeNonCharacter, latchMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "A", KeycodeNonCharacter, latchMask));
    QVERIFY(!filterKeyRelease(key, Qt::NoModifier, "", KeycodeNonCharacter, latchMask));
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
}


void Ut_MHardwareKeyboard::testAutoCaps()
{
    QSignalSpy modifierSpy(m_hkb, SIGNAL(modifierStateChanged(Qt::KeyboardModifier, ModifierState)));
    QVERIFY(modifierSpy.isValid());
    QSignalSpy shiftSpy(m_hkb, SIGNAL(shiftStateChanged()));
    QVERIFY(shiftSpy.isValid());

    // Autocaps on
    QVERIFY(!m_hkb->autoCaps);
    m_hkb->setAutoCapitalization(true);
    QVERIFY(m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, ShiftMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), 1);
    QCOMPARE(modifierSpy.at(0).at(0).value<Qt::KeyboardModifier>(), Qt::ShiftModifier);
    QCOMPARE(modifierSpy.at(0).at(1).value<ModifierState>(), ModifierLatchedState);

    // Autocaps off
    m_hkb->setAutoCapitalization(false);
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    modifierSpy.clear();
    shiftSpy.clear();

    // Autocaps on again
    m_hkb->setAutoCapitalization(true);
    QVERIFY(m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, ShiftMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), 1);
    QCOMPARE(modifierSpy.at(0).at(0).value<Qt::KeyboardModifier>(), Qt::ShiftModifier);
    QCOMPARE(modifierSpy.at(0).at(1).value<ModifierState>(), ModifierLatchedState);
    modifierSpy.clear();
    shiftSpy.clear();

    // Shift click turns autocaps off and unlatches shift (instead of normal latched -> locked)
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, ShiftMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(!filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), 1);
    QCOMPARE(modifierSpy.at(0).at(0).value<Qt::KeyboardModifier>(), Qt::ShiftModifier);
    QCOMPARE(modifierSpy.at(0).at(1).value<ModifierState>(), ModifierClearState);

    // Afterwards normal clear -> latched -> locked cycle works
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(!filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, ShiftMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(!filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, LockMask));
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, LockMask));
    QVERIFY(!filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, LockMask | ShiftMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));

    modifierSpy.clear();
    shiftSpy.clear();

    // Autocaps is ignored in [phone] number keyboard state
    m_hkb->reset();
    m_hkb->setKeyboardType(M::PhoneNumberContentType);
    int countBeforeAutoCaps = modifierSpy.count();
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));
    QCOMPARE(modifierSpy.count(), countBeforeAutoCaps);
    QCOMPARE(shiftSpy.count(), 0);
    modifierSpy.clear();

    m_hkb->reset();
    m_hkb->setKeyboardType(M::NumberContentType);
    countBeforeAutoCaps = modifierSpy.count();
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));
    QCOMPARE(modifierSpy.count(), countBeforeAutoCaps);
    QCOMPARE(shiftSpy.count(), 0);
    m_hkb->reset();
    m_hkb->setKeyboardType(M::FreeTextContentType);
    modifierSpy.clear();

    // Autocaps is ignored when Sym+ccc... is in progress
    QVERIFY(!filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    m_hkb->setAutoCapitalization(true);
    QVERIFY(!m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 0);
    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(!filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));

    // Works again now that we stopped Sym+ccc...
    m_hkb->setAutoCapitalization(true);
    QVERIFY(m_hkb->autoCaps);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, ShiftMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QCOMPARE(modifierSpy.count(), 1);
    QCOMPARE(shiftSpy.count(), 1);
    QCOMPARE(modifierSpy.at(0).at(0).value<Qt::KeyboardModifier>(), Qt::ShiftModifier);
    QCOMPARE(modifierSpy.at(0).at(1).value<ModifierState>(), ModifierLatchedState);
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
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));

    // Shift won't change the state
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(!filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask | ShiftMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));

    // Neither does Fn
    QVERIFY(!filterKeyPress(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(!filterKeyRelease(FnLevelKey, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, FnModifierMask));

    // Neither does Shift+Shift
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask));
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask | ShiftMask));
    QVERIFY(!filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask | ShiftMask));
    QVERIFY(!filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, FnModifierMask | ShiftMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
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

void Ut_MHardwareKeyboard::testShiftShiftCapsLock()
{
    QFETCH(int, state);

    setState(state);

    // Shift+Shift -> lock
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(!filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask));
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, LockMask));

    // Until we have released both shift keys, shift releases don't change state
    for (int i = 0; i < 2; ++i) {
        QVERIFY(!filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, LockMask | ShiftMask));
        QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
        QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, LockMask));
    }

    filterKeyPress(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, LockMask);
    filterKeyRelease(Qt::Key_Shift, Qt::NoModifier, "", KeycodeNonCharacter, ShiftMask);
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, 0));
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
    QTest::newRow("Fn latched") << 4 << Qt::Key_Shift << static_cast<unsigned int>(ShiftMask);
    QTest::newRow("Fn locked") << 5 << Qt::Key_Shift << static_cast<unsigned int>(ShiftMask);
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
    QVERIFY(!filterKeyPress(otherKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(!filterKeyRelease(otherKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));

    // ...we should end up to a latched state for that modifier.
    QVERIFY(checkLatchedState(ShiftMask | FnModifierMask, latchedMask));
    QVERIFY(checkLockedState(ShiftMask | LockMask | FnModifierMask, 0));
    QVERIFY(!m_hkb->autoCaps);
}


void Ut_MHardwareKeyboard::testSymClick()
{
    QSignalSpy symSpy(m_hkb, SIGNAL(symbolKeyClicked()));
    QVERIFY(symSpy.isValid());

    // Press+release gives us signal
    QVERIFY(!filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));
    QCOMPARE(symSpy.count(), 1);
    symSpy.clear();

    // Sym press + something + sym release doesn't
    QVERIFY(!filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(!filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));
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
    QVERIFY(!filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    for (int i = 0; i < iterations; ++i) {
        inputContextConnection->sendPreeditString("", PreeditNoCandidates);
        QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
        QCOMPARE(inputContextConnection->lastPreeditString().length(), 0);
        QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
        QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
        QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    }
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(result));

    QVERIFY(!filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));
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

    // SymP+a+a+o+b+SymR commits result of Sym+a+a on 'o' release...
    QVERIFY(!filterKeyPress(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, 0));
    QVERIFY(filterKeyPress(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(filterKeyRelease(Qt::Key_A, Qt::NoModifier, "a", KeycodeCharacter, SymModifierMask));
    QVERIFY(filterKeyPress(Qt::Key_O, Qt::NoModifier, "o", KeycodeCharacter, SymModifierMask));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 0);
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(QChar(0x00E4)));
    QVERIFY(filterKeyRelease(Qt::Key_O, Qt::NoModifier, "o", KeycodeCharacter, SymModifierMask));
    QCOMPARE(inputContextConnection->lastCommitString().length(), 1);
    QCOMPARE(inputContextConnection->lastCommitString(), QString(QChar(0x00E4)));
    QCOMPARE(inputContextConnection->lastPreeditString().length(), 1);
    QCOMPARE(inputContextConnection->lastPreeditString(), QString(QChar(0x00F6)));

    // ...and result of Sym+o is committed on b release.  Since b has no loops,
    // it's sent as an ordinary event.
    QVERIFY(filterKeyPress(Qt::Key_O, Qt::NoModifier, "b", KeycodeCharacter, SymModifierMask));
    QCOMPARE(inputContextConnection->lastCommitString(), QString(QChar(0x00E4)));
    QCOMPARE(inputContextConnection->keyEventsSent(), static_cast<unsigned int >(0));
    QVERIFY(filterKeyRelease(Qt::Key_O, Qt::NoModifier, "b", KeycodeCharacter, SymModifierMask));
    QCOMPARE(inputContextConnection->lastCommitString(), QString(QChar(0x00F6)));
    QCOMPARE(inputContextConnection->keyEventsSent(), static_cast<unsigned int >(1));

    QVERIFY(!filterKeyRelease(SymKey, Qt::NoModifier, "", KeycodeNonCharacter, SymModifierMask));

    QCOMPARE(symSpy.count(), 0);
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

QTEST_APPLESS_MAIN(Ut_MHardwareKeyboard);
