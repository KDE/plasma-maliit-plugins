/* * This file is part of dui-keyboard *
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



#include "ut_duihardwarekeyboard.h"
#include "duixkb_stub.h"
#include "hwkbcharloopsmanager_stub.h"
#include "duihardwarekeyboard.h"
#include <DuiApplication>
#include <duiplainwindow.h>
#include <DuiNamespace>
#include <QDebug>
#include <QSignalSpy>
#include <QKeyEvent>

namespace
{
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    const Qt::Key FnLevelKey = Qt::Key_AltGr;
    const Qt::Key SymKey = Qt::Key_Multi_key;
};

void Ut_DuiHardwareKeyboard::initTestCase()
{
    // Avoid waiting if im server is not responding
    DuiApplication::setLoadDuiInputContext(false);

    static char *argv[2] = {(char *) "ut_duihardwarekeyboard", (char *) "-software"};
    static int argc = 2;
    app = new DuiApplication(argc, argv);
}

void Ut_DuiHardwareKeyboard::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ut_DuiHardwareKeyboard::init()
{
    m_hkb = new DuiHardwareKeyboard(0);
    m_hkb->setKeyboardType(Dui::FreeTextContentType);
}

void Ut_DuiHardwareKeyboard::cleanup()
{
    delete m_hkb;
}

void Ut_DuiHardwareKeyboard::testSetModifierState()
{
    QSignalSpy mySpy(m_hkb, SIGNAL(modifierStateChanged(Qt::KeyboardModifier, ModifierState)));
    QVERIFY(mySpy.isValid());
    int modifierStateChangedSignalCount = 0;
    m_hkb->reset();
    m_hkb->setModifierState(Qt::ShiftModifier, ModifierClearState);
    QCOMPARE(mySpy.count(), ++modifierStateChangedSignalCount);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    m_hkb->setModifierState(Qt::ShiftModifier, ModifierLatchedState);
    QCOMPARE(mySpy.count(), ++modifierStateChangedSignalCount);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    m_hkb->setModifierState(Qt::ShiftModifier, ModifierLockedState);
    QCOMPARE(mySpy.count(), ++modifierStateChangedSignalCount);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLockedState);

    m_hkb->reset();
    m_hkb->setModifierState(FnLevelModifier, ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
    m_hkb->setModifierState(FnLevelModifier, ModifierLatchedState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLatchedState);
    m_hkb->setModifierState(FnLevelModifier, ModifierLockedState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLockedState);
}


void Ut_DuiHardwareKeyboard::testRedirectKey()
{
    QVERIFY(m_hkb->sensitiveKeys.count() > 0);

    foreach(const DuiHardwareKeyboard::RedirectedKey & key, m_hkb->sensitiveKeys) {
        if (key.modifier.modifier != Qt::NoModifier)
            testModifierRedirectKey(key.keyCode);
        else if (key.keyCode == SymKey)
            testSymbolRedirectKey();
    }

    m_hkb->reset();
    //the latched states are able to coexist
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLatchedState);
}

void Ut_DuiHardwareKeyboard::testModifierRedirectKey(Qt::Key modifierKey)
{
    Qt::KeyboardModifier modifier = m_hkb->keyToModifier(modifierKey);
    QVERIFY(modifier != Qt::NoModifier);

    m_hkb->reset();
    //case: modifier key press and release when clear state: clear -> latched
    QCOMPARE(m_hkb->modifierState(modifier), ModifierClearState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, modifierKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, modifierKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierLatchedState);

    //case: character key press and release when modifier key is latched: latched -> clear
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_I, Qt::NoModifier, "I", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierClearState);

    //case: key press and release for modifier key twice: clear -> latched -> locked
    m_hkb->reset();
    QCOMPARE(m_hkb->modifierState(modifier), ModifierClearState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, modifierKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, modifierKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, modifierKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, modifierKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierLockedState);

    //case: character key press won't change the lock state
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_I, Qt::NoModifier, "I", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierLockedState);

    m_hkb->filterKeyEvent(false, QEvent::KeyPress, modifierKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierLockedState);
    //case: the third time modifier key release will release lock back to clear: loched -> clear
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, modifierKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(modifier), ModifierClearState);
}

void Ut_DuiHardwareKeyboard::testSymbolRedirectKey()
{
    m_hkb->reset();
    QSignalSpy mySpy(m_hkb, SIGNAL(symbolKeyClicked()));
    QSignalSpy mySpy2(m_hkb, SIGNAL(shiftLevelChanged()));
    QVERIFY(mySpy.isValid());
    QVERIFY(mySpy2.isValid());
    //symkey to show symbol view
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, SymKey, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, SymKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(mySpy.count(), 1);

    //next symkey is to switch page
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, SymKey, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, SymKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(mySpy.count(), 2);

    //shift key when symbol view is shown
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(mySpy2.count(), 1);
}

void Ut_DuiHardwareKeyboard::testModifierInNumberContentType()
{
    m_hkb->setKeyboardType(Dui::NumberContentType);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLockedState);
    //with number content type, the FN modifier key input can not change the locked state
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLockedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLockedState);
}

void Ut_DuiHardwareKeyboard::testModifierInPhoneNumberContentType()
{
    m_hkb->setKeyboardType(Dui::PhoneNumberContentType);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLockedState);
    //with phone number content type, the FN modifier key input still can change the locked state back to clear state
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLockedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
}

void Ut_DuiHardwareKeyboard::testReset()
{
    m_hkb->reset();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    m_hkb->reset();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLockedState);
    m_hkb->reset();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
}

void Ut_DuiHardwareKeyboard::testAutoCaps()
{
    m_hkb->reset();
    QCOMPARE(m_hkb->autoCaps, false);
    m_hkb->setAutoCapitalization(true);
    // atocaps state: latched
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    QCOMPARE(m_hkb->autoCaps, true);
    //if latched state is caused by auto capitalization
    //then the shift modifier key input will change its state to lower case.
    //autoCaps -> clear
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->autoCaps, false);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    //clear -> latched
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    //latched -> locked
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLockedState);
    //locked -> clear
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
}

void Ut_DuiHardwareKeyboard::testMultiKeys()
{
    //Shift-press
    //Fn-press
    //a press release
    //Shift-release
    //Fn-release
    m_hkb->reset();
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);

    //To check Fn doesn't reset shift
    //Shift-press
    //Shift-release
    //Fn-press
    //a press-release
    //Fn-release
    m_hkb->reset();
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);

    //To check Fn should reset shift:
    //Shift-press
    //Shift-release
    //Fn-press
    //Fn-release
    //a-press-release
    m_hkb->reset();
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);

    //To check Shift should reset Fn:
    //Fn-press
    //Fn-release
    //Shift-press
    //Shift-release
    //a-press-release
    m_hkb->reset();
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);

    //To check Fn should reset shift:
    //Shift-press
    //Shift-release
    //Shift-press
    //Shift-release
    //Fn-press
    //Fn-release
    //a-press-release
    m_hkb->reset();
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLockedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLatchedState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, "a", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
}

void Ut_DuiHardwareKeyboard::testHandleIndicatorButtonClick()
{

    //default handleIndicatorButtonClick is clicking on shift button
    //"abc" -> "Abc" -> "ABC" -> "abc"
    m_hkb->reset();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
    m_hkb->handleIndicatorButtonClick();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
    m_hkb->handleIndicatorButtonClick();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLockedState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
    m_hkb->handleIndicatorButtonClick();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);

    //if fn modifier is latched, handleIndicatorButtonClick
    //"123" -> "123_" -> "abc"
    m_hkb->reset();
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLatchedState);
    m_hkb->handleIndicatorButtonClick();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLockedState);
    m_hkb->handleIndicatorButtonClick();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);

    //handleIndicatorButtonClick change shift modifier to latched state, and then receive Fn modifier click
    //"abc" -> "123" -> "123_" -> "abc"
    m_hkb->reset();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
    m_hkb->handleIndicatorButtonClick();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, FnLevelKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLatchedState);
    m_hkb->handleIndicatorButtonClick();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierLockedState);
    m_hkb->handleIndicatorButtonClick();
    QCOMPARE(m_hkb->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(m_hkb->modifierState(FnLevelModifier), ModifierClearState);
}

void Ut_DuiHardwareKeyboard::testSymbolPlusCharKeys()
{
    //set display language to en_gb
    DuiGConfItem systemDisplayLanguage(SystemDisplayLanguage);
    systemDisplayLanguage.set(QVariant("en_gb"));
    m_hkb->reset();
    QSignalSpy mySpy(m_hkb, SIGNAL(symbolCharacterKeyClicked(const QChar &, int, bool)));
    const int symIndex = m_hkb->redirectedKeyIndex(SymKey);
    QCOMPARE(m_hkb->sensitiveKeys[symIndex].pressed, false);
    QCOMPARE(m_hkb->sensitiveKeys[symIndex].charKeyClicked, false);
    QVERIFY(m_hkb->sensitiveKeys[symIndex].lastClickedCharacter.isNull());
    QCOMPARE(m_hkb->sensitiveKeys[symIndex].charKeyClickedCount, 0);

    QChar character('a');
    int pressedCount = 0;
    QString accentedCharacters = m_hkb->hwkbCharLoopsManager.characterLoop(character);
    QVERIFY(!accentedCharacters.isEmpty());

    //sym key pressed + character key pressed together
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, SymKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(m_hkb->sensitiveKeys[symIndex].pressed, true);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, character, false, 1, 0);

    //first clicking character key (when symbol key is held) will emit a symbolCharacterKeyClicked() signal
    //with the first accented character in the accented characters' loop, and not committed
    QCOMPARE(mySpy.count(), 1);
    QCOMPARE(mySpy.first().count(), 3);
    QCOMPARE(mySpy.first().at(0).value<QChar>(), accentedCharacters.at(pressedCount));
    QCOMPARE(mySpy.first().at(2).value<bool>(), false);
    ++pressedCount;
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, character, false, 1, 0);

    QCOMPARE(m_hkb->sensitiveKeys[symIndex].charKeyClicked, true);
    QCOMPARE(m_hkb->sensitiveKeys[symIndex].lastClickedCharacter, character);
    QCOMPARE(m_hkb->sensitiveKeys[symIndex].charKeyClickedCount, pressedCount);

    //next clicking of the same character, emit one symbolCharacterKeyClicked() signal
    //with the next accented character in the loop, and not committed
    while (pressedCount < accentedCharacters.length()) {
        mySpy.clear();
        m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, character, false, 1, 0);
        QCOMPARE(mySpy.count(), 1);
        QCOMPARE(mySpy.first().count(), 3);
        QCOMPARE(mySpy.first().at(0).value<QChar>(), accentedCharacters.at(pressedCount));
        QCOMPARE(mySpy.first().at(2).value<bool>(), false);
        m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, character, false, 1, 0);
        ++pressedCount;
        QCOMPARE(m_hkb->sensitiveKeys[symIndex].pressed, true);
        QCOMPARE(m_hkb->sensitiveKeys[symIndex].charKeyClicked, true);
        QCOMPARE(m_hkb->sensitiveKeys[symIndex].lastClickedCharacter, character);
        QCOMPARE(m_hkb->sensitiveKeys[symIndex].charKeyClickedCount, pressedCount);
    }

    //sym key release will emit a symbolCharacterKeyClicked() signal
    //with the last input accented character in the loop, and committed.
    mySpy.clear();
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, SymKey, Qt::NoModifier, "", false, 1, 0);
    QCOMPARE(mySpy.count(), 1);
    QCOMPARE(mySpy.first().count(), 3);
    QCOMPARE(mySpy.first().at(0).value<QChar>(), accentedCharacters.at(pressedCount - 1));
    QCOMPARE(mySpy.first().at(2).value<bool>(), true);

    //sym key release will clear the recorded states.
    QCOMPARE(m_hkb->sensitiveKeys[symIndex].pressed, false);
    QCOMPARE(m_hkb->sensitiveKeys[symIndex].charKeyClicked, false);
    QVERIFY(m_hkb->sensitiveKeys[symIndex].lastClickedCharacter.isNull());
    QCOMPARE(m_hkb->sensitiveKeys[symIndex].charKeyClickedCount, 0);

    //if different character key is clicked (when symbol key is held)
    //it will emit a symbolCharacterKeyClicked() signal to commit previous input character.
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, SymKey, Qt::NoModifier, "", false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, character, false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, character, false, 1, 0);
    mySpy.clear();
    character = QChar('b');
    accentedCharacters = m_hkb->hwkbCharLoopsManager.characterLoop(character);
    //'b' don't have accented character, so the expected input is still 'b'
    QVERIFY(accentedCharacters.isEmpty());
    m_hkb->filterKeyEvent(false, QEvent::KeyPress, Qt::Key_B, Qt::NoModifier, character, false, 1, 0);
    QCOMPARE(mySpy.count(), 2);
    QCOMPARE(mySpy.first().count(), 3);
    QCOMPARE(mySpy.first().at(0).value<QChar>(), m_hkb->hwkbCharLoopsManager.characterLoop('a').at(0));
    QCOMPARE(mySpy.first().at(2).value<bool>(), true);

    QCOMPARE(mySpy.at(1).count(), 3);
    QCOMPARE(mySpy.at(1).at(0).value<QChar>(), character);
    QCOMPARE(mySpy.at(1).at(2).value<bool>(), false);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, Qt::Key_B, Qt::NoModifier, character, false, 1, 0);
    m_hkb->filterKeyEvent(false, QEvent::KeyRelease, SymKey, Qt::NoModifier, "", false, 1, 0);
}

QTEST_APPLESS_MAIN(Ut_DuiHardwareKeyboard);
