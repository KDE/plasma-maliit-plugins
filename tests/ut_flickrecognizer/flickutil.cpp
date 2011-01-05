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

#include "flickutil.h"

#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QTest>
#include <QTime>
#include <QCoreApplication>

namespace {

    QPoint globalPos(QGraphicsObject *target, const QPointF &pos)
    {
        QGraphicsView *view = target->scene()->views().at(0);
        const QPointF scenePos(target->mapToScene(pos));
        // By default QPointF -> QPoint is done with rounding to closest integer. However we need to
        // use floor.
        return view->mapToGlobal(view->mapFromScene(scenePos - QPointF(0.49999, 0.49999)));
    }

    // QTest::qWait() is too unreliable, especially with mouse swipes
    // with multiple waits. Let's just have a busy-loop.
    void wait(int msecs, bool processEvents = false)
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

    void mousePress(QGraphicsObject *target, const QPointF &pos, int delayAfterPress = 0)
    {
        QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
        press.setPos(pos);
        press.setScreenPos(globalPos(target, pos));
        target->scene()->sendEvent(target, &press);
        wait(delayAfterPress);
    }

    void mouseMove(QGraphicsObject *target, const QPointF &pos, int delayAfterMove = 0)
    {
        QGraphicsSceneMouseEvent move(QEvent::GraphicsSceneMouseMove);
        move.setPos(pos);
        move.setScreenPos(globalPos(target, pos));
        target->scene()->sendEvent(target, &move);
        wait(delayAfterMove);
    }

    void mouseRelease(QGraphicsObject *target, const QPointF &pos, int delayAfterRelease = 0)
    {
        QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
        release.setPos(pos);
        release.setScreenPos(globalPos(target, pos));
        target->scene()->sendEvent(target, &release);
        wait(delayAfterRelease);
    }
}

void doMouseSwipe(QGraphicsObject *target, const QList<QPointF> &path, unsigned int duration)
{
    int numberOfWaitsLeft = path.count() - 1;
    int moveDelay = duration / numberOfWaitsLeft;
    int timeLeft = (duration);
    QTime time;
    time.start();

    for (int i = 0; i < path.count(); ++i) {
        if (i == 0) {
            // Simulate press
            mousePress(target, path.front(), moveDelay);
        } else if (i == (path.count() - 1)) {
            // Simulate release
            mouseRelease(target, path.back());
        } else {
            // Simulate move
            mouseMove(target, path.at(i), moveDelay);
        }

        // Adjust moveDelay if we are behind target duration.
        if (--numberOfWaitsLeft) {
            timeLeft = qMax(0, static_cast<int>(duration) - time.elapsed());
            moveDelay = timeLeft / numberOfWaitsLeft;
        }
    }
}

void doMouseSwipe(QGraphicsObject *target, const QPointF &start, const QPointF &end,
                  unsigned int duration, unsigned int intermediateSteps,
                  bool lastMoveLandsOnEnd)
{
    doMouseSwipe(target, makeSwipePointPath(start, end, intermediateSteps, lastMoveLandsOnEnd), duration);
}

QList<QPointF> makeSwipePointPath(const QPointF &start, const QPointF &end,
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
