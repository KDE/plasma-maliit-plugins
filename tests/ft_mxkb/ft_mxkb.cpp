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



#include "ft_mxkb.h"
#include <MApplication>
#include <mplainwindow.h>
#include <MNamespace>
#include <QDebug>
#include <QSignalSpy>
#include <QKeyEvent>
#include "mxkb.h"

#include <X11/X.h>
#include <X11/XKBlib.h>

namespace {
    const unsigned int FnModifierMask = Mod5Mask;
};

void Ft_MXkb::initTestCase()
{
    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);

    static char *argv[2] = {(char *) "ft_mxkb", (char *) "-software"};
    static int argc = 2;
    app = new MApplication(argc, argv);
}

void Ft_MXkb::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ft_MXkb::init()
{
    m_subject = new MXkb();
    m_subject->latchModifiers(ShiftMask | FnModifierMask, 0);
    m_subject->lockModifiers(ShiftMask | FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));
    QVERIFY(!testModifierLatchedState(FnModifierMask));
}

void Ft_MXkb::cleanup()
{
    m_subject->latchModifiers(ShiftMask | FnModifierMask, 0);
    m_subject->lockModifiers(ShiftMask | FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));
    QVERIFY(!testModifierLatchedState(FnModifierMask));
    delete m_subject;
}

void Ft_MXkb::testLatchModifiers()
{
    // latch shift
    m_subject->latchModifiers(ShiftMask, ShiftMask);
    QVERIFY(testModifierLatchedState(ShiftMask));

    // unlatch shift
    m_subject->latchModifiers(ShiftMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));

    // latch fn
    m_subject->latchModifiers(FnModifierMask, FnModifierMask);
    QVERIFY(testModifierLatchedState(FnModifierMask));

    // unlatch fn
    m_subject->latchModifiers(FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(FnModifierMask));

    // latch both
    m_subject->latchModifiers(ShiftMask | FnModifierMask, ShiftMask | FnModifierMask);
    QVERIFY(testModifierLatchedState(ShiftMask));
    QVERIFY(testModifierLatchedState(FnModifierMask));

    // unlatch both
    m_subject->latchModifiers(ShiftMask | FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));
    QVERIFY(!testModifierLatchedState(FnModifierMask));
}

void Ft_MXkb::testLockModifiers()
{
    // lock shift
    m_subject->lockModifiers(ShiftMask, ShiftMask);
    QVERIFY(testModifierLatchedState(ShiftMask));

    // unlatch shift should not work
    m_subject->latchModifiers(ShiftMask, 0);
    QVERIFY(testModifierLatchedState(ShiftMask));

    // unlock shift
    m_subject->lockModifiers(ShiftMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));

    // lock fn
    m_subject->lockModifiers(FnModifierMask, FnModifierMask);
    QVERIFY(testModifierLatchedState(FnModifierMask));

    // unlatch fn should not work
    m_subject->latchModifiers(FnModifierMask, 0);
    QVERIFY(testModifierLatchedState(FnModifierMask));

    // unlock fn
    m_subject->lockModifiers(FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(FnModifierMask));

    // lock both
    m_subject->lockModifiers(ShiftMask | FnModifierMask, ShiftMask | FnModifierMask);
    QVERIFY(testModifierLatchedState(ShiftMask));
    QVERIFY(testModifierLatchedState(FnModifierMask));

    // unlatch shift/fn should not work
    m_subject->latchModifiers(ShiftMask, 0);
    QVERIFY(testModifierLatchedState(ShiftMask));
    m_subject->latchModifiers(FnModifierMask, 0);
    QVERIFY(testModifierLatchedState(FnModifierMask));

    // unlock both
    m_subject->lockModifiers(ShiftMask | FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));
    QVERIFY(!testModifierLatchedState(FnModifierMask));
}

bool Ft_MXkb::testModifierLatchedState(int xModifier) const
{
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    unsigned int mask;
    XQueryPointer(QX11Info::display(), DefaultRootWindow(QX11Info::display()), &dummy1, &dummy2,
                  &dummy3, &dummy4, &dummy5, &dummy6, &mask);
    return (mask & xModifier);
}

QTEST_APPLESS_MAIN(Ft_MXkb);
