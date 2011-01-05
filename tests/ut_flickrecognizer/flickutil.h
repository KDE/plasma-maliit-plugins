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

#include <QPointF>
#include <QList>

class QGraphicsObject;

void doMouseSwipe(QGraphicsObject *target, const QList<QPointF> &path, unsigned int duration);

void doMouseSwipe(QGraphicsObject *target, const QPointF &start, const QPointF &end,
                  unsigned int duration, unsigned int intermediateSteps = 4,
                  bool lastMoveLandsOnEnd = false);

QList<QPointF> makeSwipePointPath(const QPointF &start, const QPointF &end,
                                  unsigned int intermediateSteps,
                                  bool lastMoveLandsOnEnd = false);

#endif // FLICKUTIL_H
