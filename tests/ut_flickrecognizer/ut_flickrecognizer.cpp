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

#include "ut_flickrecognizer.h"
#include "flickutil.h"
#include "utils.h"

#include "flickgesture.h"
#include "flickgesturerecognizer.h"

#include <MApplication>
#include <MScene>
#include <MSceneManager>
#include <MWindow>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>

Q_DECLARE_METATYPE(FlickGesture::Direction);
Q_DECLARE_METATYPE(QList<QPoint>);

class FlickTarget : public QGraphicsWidget
{
public:
    FlickTarget(QGraphicsItem *parent = 0)
        : QGraphicsWidget(parent)
    {
    }

    int numberOfFlickGestures() const
    {
        int sum = 0;
        for (int i = 0; i < 4; ++i) {
            sum += flickCountArray[i];
        }
        return sum;
    }

    int numberOfFlickStarts() const
    {
        int sum = 0;
        for (int i = 0; i < 4; ++i) {
            sum += flickStartCountArray[i];
        }
        return sum;
    }

    int numberOfFlickGestures(FlickGesture::Direction direction) const
    {
        const int index = static_cast<int>(direction);
        Q_ASSERT(index < 4);
        return flickCountArray[index];
    }

    int numberOfFlickStarts(FlickGesture::Direction direction) const
    {
        const int index = static_cast<int>(direction);
        Q_ASSERT(index < 4);
        return flickStartCountArray[index];
    }


    void resetFlickCounters()
    {
        flickCountArray[0] = 0;
        flickCountArray[1] = 0;
        flickCountArray[2] = 0;
        flickCountArray[3] = 0;
        flickStartCountArray[0] = 0;
        flickStartCountArray[1] = 0;
        flickStartCountArray[2] = 0;
        flickStartCountArray[3] = 0;
    }

protected:
    virtual bool event(QEvent *event)
    {
        if (event->type() == QEvent::Gesture) {
            QGestureEvent *ge = static_cast<QGestureEvent *>(event);
            QGesture *gesture = ge->gesture(FlickGestureRecognizer::sharedGestureType());
            FlickGesture *flickGesture = static_cast<FlickGesture *>(gesture);

            handleFlickGesture(flickGesture);
            return true;
        }
        return QGraphicsWidget::event(event);
    }

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        qDebug() << "Target Mouse Press" << event->pos() << ", " << event->screenPos();
    }
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        qDebug() << "Target Mouse Move" << event->pos() << ", " << event->screenPos();
    }
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        qDebug() << "Target Mouse Release" << event->pos() << ", " << event->screenPos();
    }


    void handleFlickGesture(FlickGesture *gesture)
    {
        if (gesture->state() == Qt::GestureFinished) {
            FlickGesture::Direction dir = gesture->direction();
            if (dir != FlickGesture::NoDirection) {
                ++flickCountArray[static_cast<int>(dir)];
            }
        } else if (gesture->state() == Qt::GestureStarted) {
            FlickGesture::Direction dir = gesture->direction();
            if (dir != FlickGesture::NoDirection) {
                ++flickStartCountArray[static_cast<int>(dir)];
            }
        }
    }

    int flickCountArray[4];
    int flickStartCountArray[4];
};

void Ut_FlickRecognizer::initTestCase()
{
    qRegisterMetaType<FlickGesture::Direction>("FlickGesture::Direction");
    qRegisterMetaType< QList<QPoint> >("QList<QPoint>");

    static int argc = 3;
    static char *app_args[3] = { (char *) "ut_flickrecognizer",
                                 (char *) "-local-theme",
                                 (char *) "-software" };

    disableQtPlugins();
    app = new MApplication(argc, app_args);

    MSceneManager *sceneMgr = new MSceneManager;
    window = new MWindow(sceneMgr, 0);
    window->scene()->setSceneRect(QRect(QPoint(0, 0), sceneMgr->visibleSceneSize()));

    target = new FlickTarget;
    window->scene()->addItem(target);
    target->setPos(0, 0);
    target->resize(864, 480);

    gestureTimeout = 300;
    gestureFinishMovementThreshold = QPoint(300, 200);
    gestureStartMovementThreshold = gestureFinishMovementThreshold * 0.5;

    if (gestureTimeout <= 0
        || gestureFinishMovementThreshold.x() > window->width()
        || gestureFinishMovementThreshold.y() > window->height()) {
        QSKIP("Flick gesture parameters prevent any flick ever being made. Skipping tests.",
              SkipAll);
    }

    FlickGestureRecognizer::registerSharedRecognizer();
    FlickGestureRecognizer::instance()->setFinishThreshold(gestureFinishMovementThreshold.x(),
                                                           gestureFinishMovementThreshold.y());
    FlickGestureRecognizer::instance()->setStartThreshold(gestureStartMovementThreshold.x(),
                                                          gestureStartMovementThreshold.y());
    FlickGestureRecognizer::instance()->setTimeout(gestureTimeout);

    Qt::GestureType gtype = FlickGestureRecognizer::sharedGestureType();
    QVERIFY(gtype & Qt::CustomGesture);
    window->viewport()->grabGesture(gtype);
    target->grabGesture(gtype);

    window->show();
    QTest::qWaitForWindowShown(window);

    // Used to have trouble with delayed show caused by remote theme. Let's still have the check.
    QVERIFY(window->isVisible());
}

void Ut_FlickRecognizer::cleanupTestCase()
{
    window->close();

    FlickGestureRecognizer::unregisterSharedRecognizer();

    delete window->scene();
    delete window;
    window = 0;
    delete app;
    app = 0;
}

void Ut_FlickRecognizer::init()
{
    target->resetFlickCounters();
}

void Ut_FlickRecognizer::cleanup()
{
}

void Ut_FlickRecognizer::testDirections_data()
{
    QTest::addColumn<QPoint>("start");
    QTest::addColumn<QPoint>("end");
    QTest::addColumn<FlickGesture::Direction>("direction");

    const int xlength = gestureFinishMovementThreshold.x() + 1;
    const int ylength = gestureFinishMovementThreshold.y() + 1;

    QTest::newRow("->") << QPoint(0, 0) << QPoint(xlength, 0) << FlickGesture::Right;
    QTest::newRow("<-") << QPoint(xlength, 0) << QPoint(0, 0) << FlickGesture::Left;
    QTest::newRow("v") << QPoint(0, 0) << QPoint(0, ylength) << FlickGesture::Down;
    QTest::newRow("^") << QPoint(0, ylength) << QPoint(0, 0) << FlickGesture::Up;
}

void Ut_FlickRecognizer::testDirections()
{
    QFETCH(QPoint, start);
    QFETCH(QPoint, end);
    QFETCH(FlickGesture::Direction, direction);

    doMouseSwipe(target, start, end, gestureTimeout / 2);

    QCOMPARE(target->numberOfFlickGestures(), 1);
    QCOMPARE(target->numberOfFlickGestures(direction), 1);
}

void Ut_FlickRecognizer::testTimeout_data()
{
    QTest::addColumn<int>("duration");
    QTest::addColumn<bool>("wasFlicked");

    QTest::newRow("valid") << qMax(0, (gestureTimeout - 50)) << true;
    QTest::newRow("invalid") << (gestureTimeout + 50) << false;
}

void Ut_FlickRecognizer::testTimeout()
{
    QFETCH(int, duration);
    QFETCH(bool, wasFlicked);

    const int expectedNumOfGestures = wasFlicked ? 1 : 0;

    // Constant start and end point for this test.
    const QPoint start(0, 0);
    const QPoint end(start + QPoint(gestureFinishMovementThreshold.x() + 2, 0));

    doMouseSwipe(target, start, end, duration);

    QCOMPARE(target->numberOfFlickGestures(), expectedNumOfGestures);
}

// comment below test cases due to MCompositor bug: NB#182701 breaks us
#if 0
void Ut_FlickRecognizer::testMovementThreshold_data()
{
    QTest::addColumn<QPoint>("start");
    QTest::addColumn<QPoint>("end");
    QTest::addColumn<bool>("wasFlicked");

    QTest::newRow("valid horizontal") << QPoint(0, 0) << QPoint(gestureFinishMovementThreshold.x() + 10, 0) << true;
    QTest::newRow("invalid horizontal") << QPoint(0, 0) << QPoint(qMax(0, (gestureFinishMovementThreshold.x() - 10)), 0) << false;
    QTest::newRow("valid vertical") << QPoint(0, 0) << QPoint(0, gestureFinishMovementThreshold.y() + 10) << true;
    QTest::newRow("invalid vertical") << QPoint(0, 0) << QPoint(0, qMax(0, (gestureFinishMovementThreshold.y() - 10))) << false;
}

void Ut_FlickRecognizer::testMovementThreshold()
{
    QFETCH(QPoint, start);
    QFETCH(QPoint, end);
    QFETCH(bool, wasFlicked);

    const int expectedNumOfGestures = wasFlicked ? 1 : 0;

    doMouseSwipe(target, start, end, gestureTimeout / 2);

    QCOMPARE(target->numberOfFlickGestures(), expectedNumOfGestures);
}

void Ut_FlickRecognizer::testStartThreshold_data()
{
    const int xlength = gestureStartMovementThreshold.x();
    const int ylength = gestureStartMovementThreshold.y();

    if (xlength >= gestureFinishMovementThreshold.x()
        || ylength >= gestureFinishMovementThreshold.y()) {
        QSKIP("Cannot recognize separate start because gesture finishes too soon.", SkipSingle);
    }

    QTest::addColumn<QPoint>("start");
    QTest::addColumn<QPoint>("end");
    QTest::addColumn<bool>("wasStarted");

    QTest::newRow("valid horizontal")   << QPoint(0, 0) << QPoint(xlength, 0)     << true;
    QTest::newRow("invalid horizontal") << QPoint(0, 0) << QPoint(xlength - 1, 0) << false;
    QTest::newRow("valid vertical")     << QPoint(0, 0) << QPoint(0, ylength)     << true;
    QTest::newRow("invalid vertical")   << QPoint(0, 0) << QPoint(0, ylength - 1) << false;
}

void Ut_FlickRecognizer::testStartThreshold()
{
    QFETCH(QPoint, start);
    QFETCH(QPoint, end);
    QFETCH(bool, wasStarted);

    const int expectedNumOfStarts = wasStarted ? 1 : 0;

    // Since start gesture is not sent it it happens on mouse release
    // we set last move point to land on the same coordinates.
    const bool lastMoveLandsOnEnd = true;
    const int steps = 5;

    doMouseSwipe(target, start, end, gestureTimeout / 2, steps, lastMoveLandsOnEnd);

    QCOMPARE(target->numberOfFlickStarts(), expectedNumOfStarts);
    QCOMPARE(target->numberOfFlickGestures(), 0);
}

void Ut_FlickRecognizer::testInvalidZigZag_data()
{
    QTest::addColumn< QList<QPoint> >("path");
    QTest::addColumn<bool>("wasFlicked");

    const unsigned int intermediatePoints = 6;
    QList<QPoint> path = makeSwipePointPath(QPoint(0, 0),
                                            QPoint(gestureFinishMovementThreshold.x(), 0),
                                            intermediatePoints);
    const QPoint point2 = path.at(2);
    const QPoint point3 = path.at(3);

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
    QFETCH(QList<QPoint>, path);
    QFETCH(bool, wasFlicked);

    doMouseSwipe(target, path, gestureTimeout / 2);

    QCOMPARE(target->numberOfFlickGestures(), (wasFlicked ? 1 : 0));
}

void Ut_FlickRecognizer::testInvalidTwoFinger_data()
{
    QTest::addColumn< QList<QPoint> >("path");
    QTest::addColumn<bool>("wasFlicked");

    // Simulate the common accidental flick-made-by-two-fingers gesture.
    QList<QPoint> path;
    path << QPoint(0, 0)
         << QPoint(1, 0)
         << QPoint(2, 0)
         << QPoint(gestureFinishMovementThreshold.x(), 0)
         << QPoint(gestureFinishMovementThreshold.x() + 1, 0)
         << QPoint(gestureFinishMovementThreshold.x() + 2, 0);

    QTest::newRow("too long jump") << path << false;

    // Make one stop in the middle.
    path[3] = QPoint(gestureFinishMovementThreshold.x() / 2, 0);
    QTest::newRow("valid flick") << path << true;
}

void Ut_FlickRecognizer::testInvalidTwoFinger()
{
    QFETCH(QList<QPoint>, path);
    QFETCH(bool, wasFlicked);

    doMouseSwipe(target, path, gestureTimeout / 2);

    QCOMPARE(target->numberOfFlickGestures(), (wasFlicked ? 1 : 0));
}
#endif


QTEST_APPLESS_MAIN(Ut_FlickRecognizer);
