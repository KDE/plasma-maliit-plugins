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

#ifndef UT_FLICKRECOGNIZER_H
#define UT_FLICKRECOGNIZER_H

#include <QtTest/QtTest>
#include <QObject>

class FlickGestureRecognizer;
class FlickTarget;
class MApplication;
class MWindow;
class QGraphicsScene;

class Ut_FlickRecognizer : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MWindow *window;
    QGraphicsScene *scene;
    FlickGestureRecognizer *subject;
    FlickTarget *target;

    int gestureTimeout;
    QPoint gestureFinishMovementThreshold;
    QPoint gestureStartMovementThreshold;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testDirections_data();
    void testDirections();
    void testTimeout_data();
    void testTimeout();
    void testMovementThreshold_data();
    void testMovementThreshold();
    void testStartThreshold_data();
    void testStartThreshold();
    void testInvalidZigZag_data();
    void testInvalidZigZag();
    void testInvalidTwoFinger();
    void testInvalidTwoFinger_data();
};


#endif // UT_FLICKRECOGNIZER_H
