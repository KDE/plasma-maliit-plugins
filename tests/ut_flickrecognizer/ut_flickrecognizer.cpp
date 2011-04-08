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

#include "ut_flickrecognizer.h"
#include "utils.h"

#include "flickgesture.h"
#include "flickgesturerecognizer.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>

Q_DECLARE_METATYPE(FlickGesture::Direction);
Q_DECLARE_METATYPE(QList<QPointF>);

namespace {
    Qt::GestureState globalGestureState = Qt::NoGesture;

    // QTest::qWait() is too unreliable, especially with mouse swipes
    // with multiple waits. Let's just have a busy-loop.
    void busyWait(int msecs, bool processEvents = false)
    {
        static QTime time;
        time.start();
        int timeLeft = msecs;

        while (time.elapsed() <= msecs) {
            if (processEvents) {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents
                                                | QEventLoop::ExcludeSocketNotifiers,
                                                timeLeft);
            }
            timeLeft -= time.elapsed();
        }
    }
}

// Only qt can set gesture state so we stub this method here.
// This works because we only use one gesture at any given moment.
Qt::GestureState QGesture::state() const
{
    return globalGestureState;
}

void Ut_FlickRecognizer::initTestCase()
{
    qRegisterMetaType<FlickGesture::Direction>("FlickGesture::Direction");
    qRegisterMetaType< QList<QPointF> >("QList<QPointF>");

    static int argc = 2;
    static char *app_args[2] = { (char *) "ut_flickrecognizer",
                                 (char *) "-software" };

    disableQtPlugins();
    app = new QApplication(argc, app_args);

    gestureTimeout = 300;
    gestureFinishMovementThreshold = QPointF(300, 200);
    gestureStartMovementThreshold = gestureFinishMovementThreshold * 0.5;

    // Registering not really needed for this test but let's see if it crashes or something.
    FlickGestureRecognizer::registerSharedRecognizer();
    FlickGestureRecognizer::instance()->setFinishThreshold(gestureFinishMovementThreshold.x(),
                                                           gestureFinishMovementThreshold.y());
    FlickGestureRecognizer::instance()->setStartThreshold(gestureStartMovementThreshold.x(),
                                                          gestureStartMovementThreshold.y());
    FlickGestureRecognizer::instance()->setTimeout(gestureTimeout);

    // Save singleton as subject.
    subject = FlickGestureRecognizer::instance();

    // Confirm custom gesture flag.
    gtype = FlickGestureRecognizer::sharedGestureType();
    QVERIFY(gtype & Qt::CustomGesture);
}

void Ut_FlickRecognizer::cleanupTestCase()
{
    FlickGestureRecognizer::unregisterSharedRecognizer();

    delete app;
    app = 0;
}

void Ut_FlickRecognizer::init()
{
    resetFlickCounters();
    globalGestureState = Qt::NoGesture;
}

void Ut_FlickRecognizer::cleanup()
{
}

void Ut_FlickRecognizer::testDirections_data()
{
    QTest::addColumn<QPointF>("start");
    QTest::addColumn<QPointF>("end");
    QTest::addColumn<FlickGesture::Direction>("direction");

    const int xlength = gestureFinishMovementThreshold.x() + 1;
    const int ylength = gestureFinishMovementThreshold.y() + 1;

    const QRectF area(0, 0, xlength, ylength);
    QTest::newRow("->") << area.topLeft() << area.topRight() << FlickGesture::Right;
    QTest::newRow("<-") << area.topRight() << area.topLeft() << FlickGesture::Left;
    QTest::newRow("v") << area.topLeft() << area.bottomLeft() << FlickGesture::Down;
    QTest::newRow("^") << area.bottomLeft() << area.topLeft() << FlickGesture::Up;
}

void Ut_FlickRecognizer::testDirections()
{
    QFETCH(QPointF, start);
    QFETCH(QPointF, end);
    QFETCH(FlickGesture::Direction, direction);

    doMouseSwipe(start, end, 0);

    QCOMPARE(numberOfFlicksFinished(), 1);
    QCOMPARE(numberOfFlicksFinished(direction), 1);
}

void Ut_FlickRecognizer::testTimeout_data()
{
    QTest::addColumn<int>("duration");
    QTest::addColumn<bool>("wasFlicked");

    QTest::newRow("valid") << qMax(0, (gestureTimeout - 100)) << true;
    QTest::newRow("invalid") << (gestureTimeout + 100) << false;
}

void Ut_FlickRecognizer::testTimeout()
{
    QFETCH(int, duration);
    QFETCH(bool, wasFlicked);

    const int expectedNumOfGestures = wasFlicked ? 1 : 0;

    // Constant start and end point for this test.
    const QPointF start(0, 0);
    const QPointF end(start + QPointF(gestureFinishMovementThreshold.x() + 2, 0));

    doMouseSwipe(start, end, duration);

    QCOMPARE(numberOfFlicksFinished(), expectedNumOfGestures);
}

void Ut_FlickRecognizer::testMovementThreshold_data()
{
    QTest::addColumn<QPointF>("start");
    QTest::addColumn<QPointF>("end");
    QTest::addColumn<bool>("wasFlicked");

    QTest::newRow("valid horizontal") << QPointF() << QPointF(gestureFinishMovementThreshold.x() + 10, 0) << true;
    QTest::newRow("invalid horizontal") << QPointF() << QPointF(qMax<qreal>(0, (gestureFinishMovementThreshold.x() - 10)), 0) << false;
    QTest::newRow("valid vertical") << QPointF() << QPointF(0, gestureFinishMovementThreshold.y() + 10) << true;
    QTest::newRow("invalid vertical") << QPointF() << QPointF(0, qMax<qreal>(0, (gestureFinishMovementThreshold.y() - 10))) << false;
}

void Ut_FlickRecognizer::testMovementThreshold()
{
    QFETCH(QPointF, start);
    QFETCH(QPointF, end);
    QFETCH(bool, wasFlicked);

    const int expectedNumOfGestures = wasFlicked ? 1 : 0;

    doMouseSwipe(start, end, 0);

    QCOMPARE(numberOfFlicksFinished(), expectedNumOfGestures);
}

void Ut_FlickRecognizer::testStartThreshold_data()
{
    const int xlength = gestureStartMovementThreshold.x();
    const int ylength = gestureStartMovementThreshold.y();

    if (xlength >= gestureFinishMovementThreshold.x()
        || ylength >= gestureFinishMovementThreshold.y()) {
        QSKIP("Cannot recognize separate start because gesture finishes too soon.", SkipSingle);
    }

    QTest::addColumn<QPointF>("start");
    QTest::addColumn<QPointF>("end");
    QTest::addColumn<bool>("wasStarted");

    QTest::newRow("valid horizontal")   << QPointF() << QPointF(xlength, 0)     << true;
    QTest::newRow("invalid horizontal") << QPointF() << QPointF(xlength - 1, 0) << false;
    QTest::newRow("valid vertical")     << QPointF() << QPointF(0, ylength)     << true;
    QTest::newRow("invalid vertical")   << QPointF() << QPointF(0, ylength - 1) << false;
}

void Ut_FlickRecognizer::testStartThreshold()
{
    QFETCH(QPointF, start);
    QFETCH(QPointF, end);
    QFETCH(bool, wasStarted);

    const int expectedNumOfStarts = wasStarted ? 1 : 0;

    // Since start gesture is not sent it it happens on mouse release
    // we set last move point to land on the same coordinates.
    const bool lastMoveLandsOnEnd = true;
    const int steps = 5;

    doMouseSwipe(start, end, 0, steps, lastMoveLandsOnEnd);

    QCOMPARE(numberOfFlicksTriggered(), expectedNumOfStarts);
    QCOMPARE(numberOfFlicksFinished(), 0);
}

void Ut_FlickRecognizer::testInvalidZigZag_data()
{
    QTest::addColumn< QList<QPointF> >("path");
    QTest::addColumn<bool>("wasFlicked");

    const unsigned int intermediatePoints = 6;
    QList<QPointF> path = makeSwipePointPath(QPointF(0, 0),
                                             QPointF(gestureFinishMovementThreshold.x(), 0),
                                             intermediatePoints);
    const QPointF point2 = path.at(2);
    const QPointF point3 = path.at(3);

    // This one is a valid flick.
    QTest::newRow("normal flick") << path << true;

    // Switch places, we have a little zigzag.
    path[2] = point3;
    path[3] = point2;
    QTest::newRow("direction changed") << path << false;

    // Restore.
    path[2] = point2;
    path[3] = point3;

    // Switch major direction at one point.
    path[2].rx() *= -1;
    QTest::newRow("major direction changed") << path << false;
}

void Ut_FlickRecognizer::testInvalidZigZag()
{
    QFETCH(QList<QPointF>, path);
    QFETCH(bool, wasFlicked);

    doMouseSwipe(path, 0);

    QCOMPARE(numberOfFlicksFinished(), (wasFlicked ? 1 : 0));
}

void Ut_FlickRecognizer::testInvalidTwoFinger_data()
{
    QTest::addColumn< QList<QPointF> >("path");
    QTest::addColumn<bool>("wasFlicked");

    // Simulate the common accidental flick-made-by-two-fingers gesture.
    QList<QPointF> path;
    path << QPointF(0, 0)
         << QPointF(1, 0)
         << QPointF(2, 0)
         << QPointF(gestureFinishMovementThreshold.x(), 0)
         << QPointF(gestureFinishMovementThreshold.x() + 1, 0)
         << QPointF(gestureFinishMovementThreshold.x() + 2, 0);

    QTest::newRow("too long jump") << path << false;

    // Make one stop in the middle.
    path[3] = QPointF(gestureFinishMovementThreshold.x() / 2, 0);
    QTest::newRow("valid flick") << path << true;
}

void Ut_FlickRecognizer::testInvalidTwoFinger()
{
    QFETCH(QList<QPointF>, path);
    QFETCH(bool, wasFlicked);

    doMouseSwipe(path, 0);

    QCOMPARE(numberOfFlicksFinished(), (wasFlicked ? 1 : 0));
}


// Flick gesture state counters

int Ut_FlickRecognizer::numberOfFlicksFinished() const
{
    int sum = 0;
    for (int i = 0; i < 4; ++i) {
        sum += flickFinishedCountArray[i];
    }
    return sum;
}

int Ut_FlickRecognizer::numberOfFlicksTriggered() const
{
    int sum = 0;
    for (int i = 0; i < 4; ++i) {
        sum += flickTriggeredCountArray[i];
    }
    return sum;
}

int Ut_FlickRecognizer::numberOfFlicksFinished(FlickGesture::Direction direction) const
{
    const int index = static_cast<int>(direction);
    Q_ASSERT(index < 4);
    return flickFinishedCountArray[index];
}

int Ut_FlickRecognizer::numberOfFlicksTriggered(FlickGesture::Direction direction) const
{
    const int index = static_cast<int>(direction);
    Q_ASSERT(index < 4);
    return flickTriggeredCountArray[index];
}

void Ut_FlickRecognizer::resetFlickCounters()
{
    flickFinishedCountArray[0] = 0;
    flickFinishedCountArray[1] = 0;
    flickFinishedCountArray[2] = 0;
    flickFinishedCountArray[3] = 0;
    flickTriggeredCountArray[0] = 0;
    flickTriggeredCountArray[1] = 0;
    flickTriggeredCountArray[2] = 0;
    flickTriggeredCountArray[3] = 0;
}


// The following are helper methods to use recognizer (almost) as Qt would.

void Ut_FlickRecognizer::recognize(FlickGesture *gesture,
                                   QEvent *event)
{
    if (globalGestureState == Qt::GestureFinished
        || globalGestureState == Qt::GestureCanceled) {
        return;
    }

    QGestureRecognizer::Result result = subject->recognize(gesture,
                                                           0,
                                                           event);
    switch (result) {
    case QGestureRecognizer::TriggerGesture:
        {
            FlickGesture::Direction dir = gesture->direction();
            if (dir != FlickGesture::NoDirection) {
                ++flickTriggeredCountArray[static_cast<int>(dir)];
            }
            if (globalGestureState == Qt::NoGesture) {
                globalGestureState = Qt::GestureStarted;
            } else {
                globalGestureState = Qt::GestureUpdated;
            }
        }
        break;
    case QGestureRecognizer::FinishGesture:
        {
            FlickGesture::Direction dir = gesture->direction();
            if (dir != FlickGesture::NoDirection) {
                ++flickFinishedCountArray[static_cast<int>(dir)];
            }
            globalGestureState = Qt::GestureFinished;
        }
        break;
    case QGestureRecognizer::CancelGesture:
        {
            if (globalGestureState != Qt::NoGesture) {
                globalGestureState = Qt::GestureCanceled;
            }
        }
        break;
    case QGestureRecognizer::Ignore:
    case QGestureRecognizer::MayBeGesture:
        break;
    default:
        globalGestureState = Qt::NoGesture;
        break;
    }
}

void Ut_FlickRecognizer::mousePress(FlickGesture *gesture,
                                    const QPointF &pos,
                                    int delayAfterPress)
{
    QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
    press.setPos(pos);
    recognize(gesture, &press);
    busyWait(delayAfterPress);
}

void Ut_FlickRecognizer::mouseMove(FlickGesture *gesture,
                                   const QPointF &pos,
                                   int delayAfterMove)
{
    QGraphicsSceneMouseEvent move(QEvent::GraphicsSceneMouseMove);
    move.setPos(pos);
    recognize(gesture, &move);
    busyWait(delayAfterMove);
}

void Ut_FlickRecognizer::mouseRelease(FlickGesture *gesture,
                                      const QPointF &pos,
                                      int delayAfterRelease)
{
    QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
    release.setPos(pos);
    recognize(gesture, &release);
    busyWait(delayAfterRelease);
}

void Ut_FlickRecognizer::doMouseSwipe(const QPointF &start,
                                      const QPointF &end,
                                      unsigned int duration,
                                      unsigned int intermediateSteps,
                                      bool lastMoveLandsOnEnd)
{
    doMouseSwipe(makeSwipePointPath(start, end,
                                    intermediateSteps,
                                    lastMoveLandsOnEnd),
                 duration);
}

void Ut_FlickRecognizer::doMouseSwipe(const QList<QPointF> &path,
                                      unsigned int duration)
{
    FlickGesture *gesture = static_cast<FlickGesture *>(subject->create(0));
    Q_ASSERT(gesture);

    int numberOfWaitsLeft = path.count() - 1;
    int moveDelay = duration / numberOfWaitsLeft;
    int timeLeft = (duration);
    QTime time;
    time.start();

    for (int i = 0; i < path.count(); ++i) {
        if (i == 0) {
            // Simulate press
            mousePress(gesture, path.front(), moveDelay);
        } else if (i == (path.count() - 1)) {
            // Simulate release
            mouseRelease(gesture, path.back());
        } else {
            // Simulate move
            mouseMove(gesture, path.at(i), moveDelay);
        }

        // Adjust moveDelay if we are behind target duration.
        if (--numberOfWaitsLeft) {
            timeLeft = qMax(0, static_cast<int>(duration) - time.elapsed());
            moveDelay = timeLeft / numberOfWaitsLeft;
        }
    }
    delete gesture;
}

QList<QPointF> Ut_FlickRecognizer::makeSwipePointPath(const QPointF &start,
                                                      const QPointF &end,
                                                      unsigned int intermediateSteps,
                                                      bool lastMoveLandsOnEnd)
{
    QList<QPointF> path;
    QPointF delta = (end - start);

    if (lastMoveLandsOnEnd && intermediateSteps < 1) {
        lastMoveLandsOnEnd = false;
    }

    delta.rx() /= static_cast<qreal>(intermediateSteps + (lastMoveLandsOnEnd ? 0 : 1));
    delta.ry() /= static_cast<qreal>(intermediateSteps + (lastMoveLandsOnEnd ? 0 : 1));

    path << start;

    QPointF pos = start;
    for (unsigned int i = 0; i < intermediateSteps; ++i) {
        pos += delta;
        path << pos;
    }

    if (lastMoveLandsOnEnd) {
        path.back() = end;
    }

    path << end;

    return path;
}

QTEST_APPLESS_MAIN(Ut_FlickRecognizer);
