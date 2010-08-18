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

#ifndef FLICKGESTURERECOGNIZER_H
#define FLICKGESTURERECOGNIZER_H

#include <QGestureRecognizer>
#include "flickgesture.h"
#include <QTime>

class QGraphicsSceneMouseEvent;

/*!
  \brief FlickGestureRecognizer recognizes currently flick/swipe gestures made by one finger.
  It is build on Qt gesture framework.

  Clients wanting to receive flick gestures must grab the gesture. Before that the recognizer
  must be registered. Currently the recognizer is a singleton and is registered via
  \a registerSharedRecognizer() and unregistered via \a unregisterSharedRecognizer().
  QGraphicsObject can be set to receive flick gestures by calling
  QGraphicsObject::grabGesture(FlickGestureRecognizer::instance()->sharedGestureType()).

  Flicking is affected by three parameters: timeout, start threshold, and finish threshold.
  Gesture enters started state when start threshold is crossed. After that cancel or finish
  event is sent to target object.
*/
class FlickGestureRecognizer : public QGestureRecognizer
{
public:
    virtual QGesture *create(QObject *target);
    virtual Result recognize(QGesture *gesture, QObject *watched, QEvent *event);
    virtual void reset(QGesture *gesture);

    static void registerSharedRecognizer();
    static Qt::GestureType sharedGestureType();
    static void unregisterSharedRecognizer();
    static FlickGestureRecognizer *instance();

    void setTimeout(int timeout);
    void setFinishThreshold(int xThreshold, int yThreshold);
    void setStartThreshold(int xThreshold, int yThreshold);

private:
    FlickGestureRecognizer();
    virtual ~FlickGestureRecognizer();

    Result recognizeInit(FlickGesture &gesture, const QPoint &pos, const QPoint &screenPos);
    Result recognizeUpdate(FlickGesture &gesture, const QPoint &pos);

    bool hasGestureTimedOut(const FlickGesture &gesture) const;
    bool hasGesturePassedThreshold(const FlickGesture &gesture, const QPoint &threshold) const;
    bool isAccidentalFlick(const FlickGesture &gesture) const;
    void updateGesture(FlickGesture &gesture, const QPoint &pos);
    Result checkGesture(const FlickGesture &gesture);
    void setMajorDirectionAndDistance(FlickGesture &gesture) const;

private:
    static FlickGestureRecognizer *sharedInstance;
    static Qt::GestureType sharedType;
    QTime time;
    int timeout;
    QPoint finishThreshold;
    QPoint startThreshold;
};

#endif // FLICKGESTURERECOGNIZER_H
