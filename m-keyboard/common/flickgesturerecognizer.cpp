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

#include "flickgesturerecognizer.h"
#include "flickgesture.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QTouchEvent>

Qt::GestureType FlickGestureRecognizer::sharedType = static_cast<Qt::GestureType>(0);
FlickGestureRecognizer *FlickGestureRecognizer::sharedInstance = 0;

namespace {
    const int DefaultTimeout = 300;
    const QPoint DefaultFinishThreshold(300, 200);
    const QPoint DefaultStartThreshold(50, 25);
}

FlickGestureRecognizer::FlickGestureRecognizer()
    : timeout(DefaultTimeout),
      finishThreshold(DefaultFinishThreshold),
      startThreshold(DefaultStartThreshold)
{
    time.start();
}

FlickGestureRecognizer::~FlickGestureRecognizer()
{

}

QGesture *FlickGestureRecognizer::create(QObject */*target*/)
{
    return new FlickGesture;
}

QGestureRecognizer::Result FlickGestureRecognizer::recognize(QGesture *gesture,
                                                             QObject */*watched*/,
                                                             QEvent *event)
{
    Result result = Ignore;

    if (!gesture || !event) {
        return result;
    }

    // We know we're dealing with FlickGesture.
    FlickGesture &flickGesture = static_cast<FlickGesture &>(*gesture);

    // We only handle GraphicsSceneMouseEvent events.
    const QGraphicsSceneMouseEvent *mouseEvent(static_cast<const QGraphicsSceneMouseEvent *>(event));

    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        result = recognizeInit(flickGesture, mouseEvent->pos().toPoint(),
                               mouseEvent->screenPos());
        break;

    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
        result = recognizeUpdate(flickGesture, mouseEvent->pos().toPoint());
        break;

    default:
        result = Ignore;
        break;
    }

    if ((event->type() == QEvent::GraphicsSceneMouseRelease
         || event->type() == QEvent::UngrabMouse)
        && result != FinishGesture
        && flickGesture.pressReceived) {
        result = CancelGesture;
    }

    return result;
}

void FlickGestureRecognizer::reset(QGesture *gesture)
{
    FlickGesture &flickGesture = static_cast<FlickGesture &>(*gesture);
    flickGesture.pressReceived = false;
    QGestureRecognizer::reset(gesture);
}

QGestureRecognizer::Result
FlickGestureRecognizer::recognizeInit(FlickGesture &gesture,
                                            const QPoint &pos,
                                            const QPoint &screenPos)
{
    // When system clock is reset the used QTime may become invalid.
    // This can happen when changing to/from daylight saving.
    // Recover just by starting again. Ongoing flick gestures
    // are most probably canceled due to this (timeout).
    if (!time.isValid()) {
        time.start();
    }

    if(gesture.state() != Qt::NoGesture) {
        reset(&gesture);
    }

    // Gesture framework finds the target with this.
    gesture.setHotSpot(screenPos);

    // Set custom gesture parameters.
    gesture.startPos = pos;
    gesture.startTime = time.elapsed();
    gesture.currentPos = gesture.startPos;
    gesture.currentTime = gesture.startTime;
    gesture.dir = FlickGesture::NoDirection;
    gesture.prevDir = FlickGesture::NoDirection;
    gesture.dist = 0;
    gesture.prevDist = 0;
    gesture.hasZigZagged = false;
    gesture.isAccidentallyFlicked = false;
    gesture.pressReceived = true;

    return MayBeGesture;
}

QGestureRecognizer::Result
FlickGestureRecognizer::recognizeUpdate(FlickGesture &gesture, const QPoint &pos)
{
    Result result = Ignore;

    if (!gesture.pressReceived) {
        return result;
    }

    updateGesture(gesture, pos);

    switch (gesture.state()) {
    case Qt::NoGesture:
        if (hasGestureTimedOut(gesture)
            || gesture.hasZigZagged) {
            // Canceling a "maybe" gesture means canceling it silently, without
            // delivering any events to target. We wont see it either anymore.
            result = CancelGesture;
        } else if (hasGesturePassedThreshold(gesture, startThreshold)
                   || hasGesturePassedThreshold(gesture, finishThreshold)) {
            // Moving from Qt::NoGesture to Qt::GestureStarted.
            result = TriggerGesture;
        } else {
            // Not yet a gesture but keep tracking.
            result = Ignore;
        }
        break;
    case Qt::GestureStarted:
        if (hasGestureTimedOut(gesture)
            || gesture.hasZigZagged
            || gesture.isAccidentallyFlicked) {
            result = CancelGesture;
        } else if (hasGesturePassedThreshold(gesture, finishThreshold)) {
            result = FinishGesture;
        } else {
            result = Ignore;
        }
        break;
    default:
        qWarning() << "Flick gesture recognition is in corrupted state!\n"
                      "Current gesture state: " << gesture.state();
        break;
    }

    return result;
}

bool FlickGestureRecognizer::hasGestureTimedOut(const FlickGesture &gesture) const
{
    return gesture.elapsedTime() >= timeout;
}

bool FlickGestureRecognizer::hasGesturePassedThreshold(const FlickGesture &gesture,
                                                       const QPoint &threshold) const
{
    if (gesture.direction() == FlickGesture::NoDirection) {
        return false;
    } else {
        const bool horizontal = gesture.direction() == FlickGesture::Left
                                || gesture.direction() == FlickGesture::Right;
        const int distance = horizontal ? threshold.x() : threshold.y();
        return gesture.distance() >= distance;
    }
}

bool FlickGestureRecognizer::isAccidentalFlick(const FlickGesture &gesture) const
{
    if (!hasGesturePassedThreshold(gesture, finishThreshold)) {
        return false;
    }

    // To detect accidental two-finger swipes we limit the allowed distance traveled
    // on a single udpate.
    const int movedOnLastUpdate = qAbs(gesture.dist - gesture.prevDist);
    const qreal allowedMovementRatio = 0.8;
    const int allowedMovement = static_cast<int>(allowedMovementRatio * static_cast<qreal>(gesture.dist));
    return movedOnLastUpdate > allowedMovement;
}

void FlickGestureRecognizer::updateGesture(FlickGesture &gesture,
                                           const QPoint &pos)
{
    const int currentTime = time.elapsed();

    // In case QTime::elapsed() has wrapped, reset start time.
    if (currentTime < gesture.startTime) {
        const int wrapTime = 24 * 3600 * 1000; // 24 hours in msecs
        gesture.startTime = wrapTime - gesture.startTime;
    }

    gesture.currentTime = currentTime;
    gesture.currentPos = pos;

    setMajorDirectionAndDistance(gesture);

    if (!gesture.hasZigZagged) {
        const bool majorDirectionChanged = gesture.prevDir != FlickGesture::NoDirection
                                           && gesture.prevDir != gesture.dir;
        const bool distShortened = (gesture.dist - gesture.prevDist) < 0 ? true : false;
        gesture.hasZigZagged = majorDirectionChanged || distShortened;
    }
    
    if (!gesture.isAccidentallyFlicked) {
        gesture.isAccidentallyFlicked = isAccidentalFlick(gesture);
    }
}

void FlickGestureRecognizer::setMajorDirectionAndDistance(FlickGesture &gesture) const
{
    const QPoint diff(gesture.currentPos - gesture.startPos);
    const int horizontalDistance = qAbs(diff.x());
    const int verticalDistance = qAbs(diff.y());

    gesture.prevDir = gesture.dir;
    gesture.prevDist = gesture.dist;

    if (horizontalDistance > verticalDistance) {
        gesture.dir = diff.x() > 0 ? FlickGesture::Right : FlickGesture::Left;
        gesture.dist = horizontalDistance;
    } else if (horizontalDistance < verticalDistance) {
        gesture.dir = diff.y() > 0 ? FlickGesture::Down : FlickGesture::Up;
        gesture.dist = verticalDistance;
    }
}

void FlickGestureRecognizer::registerSharedRecognizer()
{
    if (!sharedInstance) {
        sharedInstance = new FlickGestureRecognizer;

        // Ownership of recognizer is transferred to the application.
        sharedType = registerRecognizer(sharedInstance);
    }
}

Qt::GestureType FlickGestureRecognizer::sharedGestureType()
{
    return sharedType;
}

void FlickGestureRecognizer::unregisterSharedRecognizer()
{
    if (sharedInstance) {
        unregisterRecognizer(sharedType);
        sharedType = static_cast<Qt::GestureType>(0);

        // instance owned by application
        sharedInstance = 0;
    }
}

FlickGestureRecognizer *FlickGestureRecognizer::instance()
{
    return sharedInstance;
}


void FlickGestureRecognizer::setTimeout(int timeout)
{
    this->timeout = timeout;
}

void FlickGestureRecognizer::setFinishThreshold(int xThreshold, int yThreshold)
{
    finishThreshold = QPoint(xThreshold, yThreshold);
}

void FlickGestureRecognizer::setStartThreshold(int xThreshold, int yThreshold)
{
    startThreshold = QPoint(xThreshold, yThreshold);
}
