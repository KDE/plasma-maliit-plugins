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



#ifndef UT_DUIVIRTUALKEYBOARDHOST_H
#define UT_DUIVIRTUALKEYBOARDHOST_H

#include <QtTest/QtTest>
#include <QObject>

#include <duinamespace.h>

class DuiApplication;
class DuiKeyboardHost;
class DuiInputContextStubConnection;
class DuiWindow;

class Ut_DuiKeyboardHost : public QObject
{
    Q_OBJECT
private:
    DuiApplication *app;
    DuiKeyboardHost *subject;
    DuiInputContextStubConnection *inputContext;
    DuiWindow *window;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testCreate();
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

private:
    void rotateToAngle(Dui::OrientationAngle);
};

#endif
