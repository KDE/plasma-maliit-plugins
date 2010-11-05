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

#ifndef UT_HORIZONTALSWITCHER_H
#define UT_HORIZONTALSWITCHER_H

#include <QtTest/QtTest>
#include <QObject>

class QApplication;
class HorizontalSwitcher;

class MImAbstractKeyArea
    : public QObject
{
    Q_OBJECT

public:
    void modifiersChanged(bool, QChar ch = QChar())
    {
        Q_UNUSED(ch);
    }
};


class Ut_HorizontalSwitcher : public QObject
{
    Q_OBJECT
private:
    QApplication *app;
    HorizontalSwitcher *subject;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testAddWidgets();
    void testSetCurrent_data();
    void testSetCurrent();
    void testShowWidgets();
    void testSwitchLeftRight_data();
    void testSwitchLeftRight();
    void testDisabledDuringTransitions_data();
    void testDisabledDuringTransitions();
// comment below test cases due to MCompositor bug: NB#182701 breaks us
#if 0
    void testSwitchSignals();
#endif
    void testInitialSwitchTo();
    void testIsAtBoundary();
};

#endif // UT_HORIZONTALSWITCHER_H
