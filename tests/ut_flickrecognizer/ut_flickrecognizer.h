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

#include "flickgesture.h"

class FlickGestureRecognizer;
class QApplication;

class Ut_FlickRecognizer : public QObject
{
    Q_OBJECT
private:
    QApplication *app;
    FlickGestureRecognizer *subject;
    Qt::GestureType gtype;

    int gestureTimeout;
    QPointF gestureFinishMovementThreshold;
    QPointF gestureStartMovementThreshold;

    int flickTriggeredCountArray[4];
    int flickFinishedCountArray[4];

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

private:
    int numberOfFlicksFinished() const;
    int numberOfFlicksTriggered() const;
    int numberOfFlicksFinished(FlickGesture::Direction direction) const;
    int numberOfFlicksTriggered(FlickGesture::Direction direction) const;
    void resetFlickCounters();
    void recognize(FlickGesture *gesture,
                   QEvent *event);
    void mousePress(FlickGesture *gesture,
                    const QPointF &pos,
                    int delayAfterPress = 0);
    void mouseMove(FlickGesture *gesture,
                   const QPointF &pos,
                   int delayAfterMove = 0);
    void mouseRelease(FlickGesture *gesture,
                      const QPointF &pos,
                      int delayAfterRelease = 0);
    void doMouseSwipe(const QPointF &start,
                      const QPointF &end,
                      unsigned int duration = 0,
                      unsigned int intermediateSteps = 4,
                      bool lastMoveLandsOnEnd = false);
    void doMouseSwipe(const QList<QPointF> &path,
                      unsigned int duration);
    QList<QPointF> makeSwipePointPath(const QPointF &start,
                                      const QPointF &end,
                                      unsigned int intermediateSteps,
                                      bool lastMoveLandsOnEnd = false);
};


#endif // UT_FLICKRECOGNIZER_H
