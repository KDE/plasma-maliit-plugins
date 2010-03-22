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



#include "ft_duihardwarekeyboard.h"
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

void Ft_DuiHardwareKeyboard::initTestCase()
{
    // Avoid waiting if im server is not responding
    DuiApplication::setLoadDuiInputContext(false);

    static char *argv[2] = {(char *) "ft_duihardwarekeyboard", (char *) "-software"};
    static int argc = 2;
    app = new DuiApplication(argc, argv);
}

void Ft_DuiHardwareKeyboard::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ft_DuiHardwareKeyboard::init()
{
    m_hkb = new DuiHardwareKeyboard(0, 0);
    m_hkb->setKeyboardType(Dui::FreeTextContentType);
}

void Ft_DuiHardwareKeyboard::cleanup()
{
    delete m_hkb;
}

void Ft_DuiHardwareKeyboard::testRedirectKey()
{
    testModifierRedirectKey(Qt::ShiftModifier, Qt::Key_Shift);
    testModifierRedirectKey(FnLevelModifier, FnLevelKey);
}

void Ft_DuiHardwareKeyboard::testModifierRedirectKey(Qt::KeyboardModifier modifier, Qt::Key modifierKey)
{
    m_hkb->reset();
    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), false);
    //case: shift key press and release when clear state: clear -> latched
    m_hkb->redirectKey(QEvent::KeyPress, modifierKey, "");

    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), true);
    m_hkb->redirectKey(QEvent::KeyRelease, modifierKey, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), true);

    //case: character key press and release when modifier key is latched: latched -> clear
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_I, "i");
    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), false);

    //case: key press and release for modifier key twice: clear -> latched -> locked
    m_hkb->reset();
    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), false);
    m_hkb->redirectKey(QEvent::KeyPress, modifierKey, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), true);
    m_hkb->redirectKey(QEvent::KeyRelease, modifierKey, "");
    m_hkb->redirectKey(QEvent::KeyPress, modifierKey, "");
    m_hkb->redirectKey(QEvent::KeyRelease, modifierKey, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), true);

    //case: character key press won't change the lock state
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_I, "i");
    m_hkb->redirectKey(QEvent::KeyRelease, Qt::Key_I, "i");
    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), true);

    m_hkb->redirectKey(QEvent::KeyPress, modifierKey, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), true);
    //case: the third time modifier key release will release lock back to clear: locked -> clear
    m_hkb->redirectKey(QEvent::KeyRelease, modifierKey, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(modifier), false);
}

void Ft_DuiHardwareKeyboard::testSetKeyboardType()
{
    m_hkb->reset();
    m_hkb->setKeyboardType(Dui::NumberContentType);
    QCOMPARE(m_hkb->duiXkb.isLatched(FnLevelModifier), true);
    //with number content type, the FN modifier key input still can change the locked state back to clear state
    m_hkb->redirectKey(QEvent::KeyPress, FnLevelKey, "");
    m_hkb->redirectKey(QEvent::KeyRelease, FnLevelKey, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(FnLevelModifier), false);

    m_hkb->setKeyboardType(Dui::PhoneNumberContentType);
    QCOMPARE(m_hkb->duiXkb.isLatched(FnLevelModifier), true);
    //with number content type, the FN modifier key input still can change the locked state back to clear state
    m_hkb->redirectKey(QEvent::KeyPress, FnLevelKey, "");
    m_hkb->redirectKey(QEvent::KeyRelease, FnLevelKey, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(FnLevelModifier), false);
}

void Ft_DuiHardwareKeyboard::testReset()
{
    m_hkb->reset();
    //reset is for both shift and fn modifier
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), false);
    QCOMPARE(m_hkb->duiXkb.isLatched(FnLevelModifier), false);
    //reset latched state modifiers
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_Shift, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), true);
    m_hkb->reset();
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), false);
    //reset locked state modifiers
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_Shift, "");
    m_hkb->redirectKey(QEvent::KeyRelease, Qt::Key_Shift, "");
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_Shift, "");
    m_hkb->redirectKey(QEvent::KeyRelease, Qt::Key_Shift, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), true);
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_I, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), true);
    m_hkb->reset();
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), false);
}

void Ft_DuiHardwareKeyboard::testAutoCaps()
{
    m_hkb->reset();
    m_hkb->setAutoCapitalization(true);
    // atocaps state: shift is latched
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), true);
    //if latched state is caused by auto capitalization
    //then the shift modifier key input will change its state to lower case.
    //autoCaps -> clear
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_Shift, "");
    m_hkb->redirectKey(QEvent::KeyRelease, Qt::Key_Shift, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), false);
    //clear -> latched
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_Shift, "");
    m_hkb->redirectKey(QEvent::KeyRelease, Qt::Key_Shift, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), true);
    //latched -> locked
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_Shift, "");
    m_hkb->redirectKey(QEvent::KeyRelease, Qt::Key_Shift, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), true);
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_I, "i");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), true);
    //locked -> clear
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_Shift, "");
    m_hkb->redirectKey(QEvent::KeyRelease, Qt::Key_Shift, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), false);
}

void Ft_DuiHardwareKeyboard::testMultiKeys()
{
    //shift and fn can be latched together
    m_hkb->reset();
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_Shift, "");
    m_hkb->redirectKey(QEvent::KeyRelease, Qt::Key_Shift, "");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), true);

    m_hkb->redirectKey(QEvent::KeyPress, FnLevelKey, "");
    m_hkb->redirectKey(QEvent::KeyRelease, FnLevelKey, "");

    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), false);
    QCOMPARE(m_hkb->duiXkb.isLatched(FnLevelModifier), true);

    //character input will clear all latched modifier
    m_hkb->redirectKey(QEvent::KeyPress, Qt::Key_I, "i");
    m_hkb->redirectKey(QEvent::KeyRelease, Qt::Key_I, "i");
    QCOMPARE(m_hkb->duiXkb.isLatched(Qt::ShiftModifier), false);
    QCOMPARE(m_hkb->duiXkb.isLatched(FnLevelModifier), false);
}

QTEST_APPLESS_MAIN(Ft_DuiHardwareKeyboard);
