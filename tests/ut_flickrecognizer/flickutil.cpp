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
#include <QGraphicsWidget>
#include <QTest>


void wait(int msecs)
{
    if (msecs > 10) {
        QTest::qWait(msecs);
    }
}

void doMouseSwipe(QGraphicsObject *target, const QList<QPoint> &path, unsigned int duration)
{
    const unsigned int moveDelay = duration / (path.count() - 1);

    // Simulate press
    mousePress(target, path.front(), moveDelay);

    // Simulate move
    for (int i = 1; i < path.count() - 1; ++i) {
        mouseMove(target, path.at(i), moveDelay);
    }

    // Simulate release
    mouseRelease(target, path.back());
}

void doMouseSwipe(QGraphicsObject *target, const QPoint &start, const QPoint &end,
                  unsigned int duration, unsigned int intermediateSteps,
                  bool lastMoveLandsOnEnd)
{
    doMouseSwipe(target, makeSwipePointPath(start, end, intermediateSteps, lastMoveLandsOnEnd), duration);
}

void mousePress(QGraphicsObject *target, const QPoint &pos, int delay)
{
    QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
    press.setPos(pos);
    // Not exactly the correct way to get screen pos but works for us.
    press.setScreenPos(target->mapToScene(pos).toPoint());
    target->scene()->sendEvent(target, &press);
    wait(delay);
}

void mouseMove(QGraphicsObject *target, const QPoint &pos, int delay)
{
    QGraphicsSceneMouseEvent move(QEvent::GraphicsSceneMouseMove);
    move.setPos(pos);
    move.setScreenPos(target->mapToScene(pos).toPoint());
    target->scene()->sendEvent(target, &move);
    wait(delay);
}

void mouseRelease(QGraphicsObject *target, const QPoint &pos, int delay)
{
    QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
    release.setPos(pos);
    release.setScreenPos(target->mapToScene(pos).toPoint());
    target->scene()->sendEvent(target, &release);
    wait(delay);
}

QList<QPoint> makeSwipePointPath(const QPoint &start, const QPoint &end,
                                 unsigned int intermediateSteps,
                                 bool lastMoveLandsOnEnd)
{
    QList<QPoint> path;
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
        path << pos.toPoint();
    }

    if (lastMoveLandsOnEnd) {
        path.back() = end;
    }

    path << end;

    return path;
}
