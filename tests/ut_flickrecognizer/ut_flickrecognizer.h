/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
