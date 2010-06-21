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

#ifndef FLICKUTIL_H
#define FLICKUTIL_H

#include <QPoint>
#include <QList>

class QGraphicsObject;

void doMouseSwipe(QGraphicsObject *target, const QList<QPoint> &path, unsigned int duration);

void doMouseSwipe(QGraphicsObject *target, const QPoint &start, const QPoint &end,
                  unsigned int duration, unsigned int intermediateSteps = 4,
                  bool lastMoveLandsOnEnd = false);

void mousePress(QGraphicsObject *target, const QPoint &pos, int delayAfterPress = 0);
void mouseMove(QGraphicsObject *target, const QPoint &pos, int delayAfterMove = 0);
void mouseRelease(QGraphicsObject *target, const QPoint &pos, int delayAfterRelease = 0);

QList<QPoint> makeSwipePointPath(const QPoint &start, const QPoint &end,
                                 unsigned int intermediateSteps,
                                 bool lastMoveLandsOnEnd = false);

#endif // FLICKUTIL_H
