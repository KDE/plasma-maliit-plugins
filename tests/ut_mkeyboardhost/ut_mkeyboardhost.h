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



#ifndef UT_MVIRTUALKEYBOARDHOST_H
#define UT_MVIRTUALKEYBOARDHOST_H

#include <QList>
#include <QtTest/QtTest>
#include <QObject>

#include <mnamespace.h>

class MApplication;
class MKeyboardHost;
class MInputContextStubConnection;
class MWindow;

class Ut_MKeyboardHost : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MKeyboardHost *subject;
    MInputContextStubConnection *inputContext;
    MWindow *window;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testCreate();
    void testOrientationAngleLocked();
    void testRotatePoint();
    void testRotateRect();
    void testHandleClick();
    void testDirectMode();
    void testNotCrash();
    void testErrorCorrectionOption();

    void testAutoCaps();
    void testApplicationOrientationChanged();

    void testCopyPaste();
    void testPlusMinus();

    void testRegionSignals();

    void testSetState_data();
    void testSetState();
    void testSetStateCombination();

    void testSymbolKeyClick();
    void testUpdateSymbolViewLevel();

    void testKeyCycle_data();
    void testKeyCycle();

    void testShiftState_data();
    void testShiftState();

    void testCommitPreeditOnStateChange();

    void testLayoutMenuKeyClick_data();
    void testLayoutMenuKeyClick();

    void testShiftStateOnFocusChanged_data();
    void testShiftStateOnFocusChanged();

    void testShiftStateOnLayoutChanged_data();
    void testShiftStateOnLayoutChanged();

    void testToolbar();

    void testHandleHwKeyboardStateChanged_data();
    void testHandleHwKeyboardStateChanged();

    void testUserHide();

    void testWYTIWYSErrorCorrection();

    void testSignalsInNormalMode();
    void testSignalsInDirectMode();

    void testShowLanguageNotification_data();
    void testShowLanguageNotification();

private:
    void rotateToAngle(M::OrientationAngle);
    void triggerAutoCaps();
    void testSignals(M::InputMethodMode inputMode);

public:
    enum TestOperation {
        ClickShift,
        ClickCharacter,
        ClickCharacterWithShiftDown,
        TriggerAutoCaps
    };

    typedef QList<TestOperation> TestOpList;
};

#endif
