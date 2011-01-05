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
#include <MSceneWindow>

Q_DECLARE_METATYPE(FlickGesture::Direction);
Q_DECLARE_METATYPE(QList<QPointF>);

class FlickTarget : public MSceneWindow
{
public:
    FlickTarget(QGraphicsItem *parent = 0)
        : MSceneWindow(parent)
    {
        resetFlickCounters();
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
            if (gesture) {
                FlickGesture *flickGesture = static_cast<FlickGesture *>(gesture);
                handleFlickGesture(flickGesture);
            } else {
                qWarning() << "Test received an unkown gesture";
            }
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
    qRegisterMetaType< QList<QPointF> >("QList<QPointF>");

    static int argc = 2;
    static char *app_args[2] = { (char *) "ut_flickrecognizer",
                                 (char *) "-software" };

    disableQtPlugins();
    app = new MApplication(argc, app_args);

    MSceneManager *sceneMgr = new MSceneManager;
    window = new MWindow(sceneMgr, 0);
    window->scene()->setSceneRect(QRectF(QPointF(0, 0), sceneMgr->visibleSceneSize()));

    window->setOrientationAngleLocked(true);

    gestureTimeout = 300;
    gestureFinishMovementThreshold = QPointF(300, 200);
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

    gtype = FlickGestureRecognizer::sharedGestureType();
    QVERIFY(gtype & Qt::CustomGesture);
    window->viewport()->grabGesture(gtype);

    setCustomCompositorRegion(window);

    QApplication::setActiveWindow(window);
    window->show();
    QTest::qWaitForWindowShown(window);

    // Used to have trouble with delayed show caused by remote theme. Let's still have the check.
    QVERIFY(window->isVisible());
    QVERIFY(window->testAttribute(Qt::WA_Mapped));
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
    window->sceneManager()->setOrientationAngle(M::Angle0, MSceneManager::ImmediateTransition);

    target = new FlickTarget;
    target->resize(window->sceneRect().size());
    window->sceneManager()->appearSceneWindowNow(target);
    target->grabGesture(gtype);
    QCOMPARE(target->sceneWindowState(), MSceneWindow::Appeared);
}

void Ut_FlickRecognizer::cleanup()
{
    target->ungrabGesture(gtype);
    delete target;
    target = 0;
}

void Ut_FlickRecognizer::testDirections_data()
{
    QTest::addColumn<QPointF>("start");
    QTest::addColumn<QPointF>("end");
    QTest::addColumn<FlickGesture::Direction>("direction");

    const int xlength = gestureFinishMovementThreshold.x() + 1;
    const int ylength = gestureFinishMovementThreshold.y() + 1;

    // Points where x or y is 0.0 cannot be used since they are not guaranteed to hit target.
    // Even if they did in default orientation, they will not in another orientation, i.e.
    // y=0.0 can become y=480.0 which will be out of target.
    const QRectF area(0.1, 0.1, xlength, ylength);
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

    QList<M::OrientationAngle> angles;
    angles << M::Angle0 << M::Angle90 << M::Angle180 << M::Angle270;

    foreach (M::OrientationAngle angle, angles) {

        window->sceneManager()->setOrientationAngle(angle, MSceneManager::ImmediateTransition);

        doMouseSwipe(target, start, end, 0);

        QCOMPARE(target->numberOfFlickGestures(), 1);
        QCOMPARE(target->numberOfFlickGestures(direction), 1);
        target->resetFlickCounters();
    }
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
    const QPointF start(0.1, 0.1);
    const QPointF end(start + QPointF(gestureFinishMovementThreshold.x() + 2, 0));

    doMouseSwipe(target, start, end, duration);

    QCOMPARE(target->numberOfFlickGestures(), expectedNumOfGestures);
}

void Ut_FlickRecognizer::testMovementThreshold_data()
{
    QTest::addColumn<QPointF>("start");
    QTest::addColumn<QPointF>("end");
    QTest::addColumn<bool>("wasFlicked");

    QTest::newRow("valid horizontal") << QPointF(0.1, 0.1) << QPointF(gestureFinishMovementThreshold.x() + 10, 0.1) << true;
    QTest::newRow("invalid horizontal") << QPointF(0.1, 0.1) << QPointF(qMax<qreal>(0.1, (gestureFinishMovementThreshold.x() - 10)), 0.1) << false;
    QTest::newRow("valid vertical") << QPointF(0.1, 0.1) << QPointF(0.1, gestureFinishMovementThreshold.y() + 10) << true;
    QTest::newRow("invalid vertical") << QPointF(0.1, 0.1) << QPointF(0.1, qMax<qreal>(0.1, (gestureFinishMovementThreshold.y() - 10))) << false;
}

void Ut_FlickRecognizer::testMovementThreshold()
{
    QFETCH(QPointF, start);
    QFETCH(QPointF, end);
    QFETCH(bool, wasFlicked);

    const int expectedNumOfGestures = wasFlicked ? 1 : 0;

    doMouseSwipe(target, start, end, 0);

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

    QTest::addColumn<QPointF>("start");
    QTest::addColumn<QPointF>("end");
    QTest::addColumn<bool>("wasStarted");

    QTest::newRow("valid horizontal")   << QPointF(0.1, 0.1) << QPointF(xlength, 0.1)     << true;
    QTest::newRow("invalid horizontal") << QPointF(0.1, 0.1) << QPointF(xlength - 1, 0.1) << false;
    QTest::newRow("valid vertical")     << QPointF(0.1, 0.1) << QPointF(0.1, ylength)     << true;
    QTest::newRow("invalid vertical")   << QPointF(0.1, 0.1) << QPointF(0.1, ylength - 1) << false;
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

    doMouseSwipe(target, start, end, 0, steps, lastMoveLandsOnEnd);

    QCOMPARE(target->numberOfFlickStarts(), expectedNumOfStarts);
    QCOMPARE(target->numberOfFlickGestures(), 0);
}

void Ut_FlickRecognizer::testInvalidZigZag_data()
{
    QTest::addColumn< QList<QPointF> >("path");
    QTest::addColumn<bool>("wasFlicked");

    const unsigned int intermediatePoints = 6;
    QList<QPointF> path = makeSwipePointPath(QPointF(0.1, 0.1),
                                             QPointF(gestureFinishMovementThreshold.x(), 0.1),
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

    doMouseSwipe(target, path, 0);

    QCOMPARE(target->numberOfFlickGestures(), (wasFlicked ? 1 : 0));
}

void Ut_FlickRecognizer::testInvalidTwoFinger_data()
{
    QTest::addColumn< QList<QPointF> >("path");
    QTest::addColumn<bool>("wasFlicked");

    // Simulate the common accidental flick-made-by-two-fingers gesture.
    QList<QPointF> path;
    path << QPointF(0.1, 0.1)
         << QPointF(1, 0.1)
         << QPointF(2, 0.1)
         << QPointF(gestureFinishMovementThreshold.x(), 0.1)
         << QPointF(gestureFinishMovementThreshold.x() + 1, 0.1)
         << QPointF(gestureFinishMovementThreshold.x() + 2, 0.1);

    QTest::newRow("too long jump") << path << false;

    // Make one stop in the middle.
    path[3] = QPointF(gestureFinishMovementThreshold.x() / 2, 0.1);
    QTest::newRow("valid flick") << path << true;
}

void Ut_FlickRecognizer::testInvalidTwoFinger()
{
    QFETCH(QList<QPointF>, path);
    QFETCH(bool, wasFlicked);

    doMouseSwipe(target, path, 0);

    QCOMPARE(target->numberOfFlickGestures(), (wasFlicked ? 1 : 0));
}


QTEST_APPLESS_MAIN(Ut_FlickRecognizer);
