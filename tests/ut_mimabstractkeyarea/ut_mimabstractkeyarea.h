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

#ifndef UT_MIMABSTRACTKEYAREA_H
#define UT_MIMABSTRACTKEYAREA_H

#include <QList>
#include <QObject>
#include <QtTest/QtTest>

#include "mnamespace.h"
#include "flickgesture.h"

class MApplication;
class MImAbstractKeyArea;
class KeyboardData;
class MImAbstractKey;
class MSceneWindow;
class MImAbstractKeyAreaStyle;

class Ut_MImAbstractKeyArea : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MImAbstractKeyArea *subject;
    KeyboardData *keyboard;
    MSceneWindow *sceneWindow;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testLandscapeBoxSize_data();
    void testLandscapeBoxSize();
    void testPortraitBoxSize_data();
    void testPortraitBoxSize();
    void testSceneEvent_data();
    void testSceneEvent();
    void testPaint_data();
    void testPaint();
    void testDeadkeys_data();
    void testDeadkeys();
    void testSelectedDeadkeys();
    void testTwoDeadInOne_data();
    void testTwoDeadInOne();
    void testExtendedLabels();
    void testKeyId();
    void testContentType_data();
    void testContentType();
    void testImportedLayouts_data();
    void testImportedLayouts();
    void testPopup_data();
    void testPopup();
    void testInitialization_data();
    void testInitialization();
    void testShiftCapsLock();
    void testOverridenKey_data();
    void testOverridenKey();
    void testRtlKeys_data();
    void testRtlKeys();
    void testLongKeyPress();
    void testKeyLayout();

    void testTouchPoints_data();
    void testTouchPoints();

    void testReset();

    void testStyleModesFromKeyCount_data();
    void testStyleModesFromKeyCount();

    void testOverlayMode_data();
    void testOverlayMode();

private:
    void changeOrientation(M::OrientationAngle angle);
    QSize defaultLayoutSize();

    MImAbstractKey *keyAt(unsigned int row,
                      unsigned int column) const;

    static MImAbstractKeyArea *createArea(const QString &labels,
                                          const QSize &size,
                                          const QSize &fixedNormalKeySize = QSize(48, 48),
                                          bool usePopup = false);

public:
    enum TestOperation {
        TestOpClickDeadKey,
        TestOpSetShiftOn,
        TestOpSetShiftOff
    };

    typedef QList<TestOperation> TestOpList;
};

#endif // UT_MIMABSTRACTKEYAREA_H
