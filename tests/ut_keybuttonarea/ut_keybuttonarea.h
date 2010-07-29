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

#ifndef UT_KEYBUTTONAREA_H
#define UT_KEYBUTTONAREA_H

#include <QList>
#include <QObject>
#include <QtTest/QtTest>

#include "mnamespace.h"
#include "flickgesture.h"

class MApplication;
class MVirtualKeyboardStyleContainer;
class KeyButtonArea;
class KeyboardData;
class IKeyButton;

class Ut_KeyButtonArea : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MVirtualKeyboardStyleContainer *style;
    KeyButtonArea *subject;
    KeyboardData *keyboard;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testLandscapeBoxSize_data();
    void testLandscapeBoxSize();
    void testPortraitBoxSize_data();
    void testPortraitBoxSize();
    void testLabelPosition_data();
    void testLabelPosition();
    void testFlickCheck_data();
    void testFlickCheck();
    void testSceneEvent_data();
    void testSceneEvent();
    void testPaint_data();
    void testPaint();
    void testDeadkeys_data();
    void testDeadkeys();
    void testSelectedDeadkeys();
    void testTwoDeadInOne_data();
    void testTwoDeadInOne();
    void testImportedLayouts_data();
    void testImportedLayouts();
    void testPopup_data();
    void testPopup();
    void testInitialization_data();
    void testInitialization();
    void testFunctionRowAlignmentBug_data();
    void testFunctionRowAlignmentBug();
    void testShiftCapsLock();
    void testMultiTouch();
    void testRtlKeys_data();
    void testRtlKeys();

private:
    void changeOrientation(M::OrientationAngle angle);
    QSize defaultLayoutSize();

    IKeyButton *keyAt(unsigned int row, unsigned int column) const;

public:
    enum TestOperation {
        TestOpClickDeadKey,
        TestOpSetShiftOn,
        TestOpSetShiftOff
    };

    typedef QList<TestOperation> TestOpList;
};

#endif // UT_KEYBUTTONAREA_H
