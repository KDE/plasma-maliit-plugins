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
#include "touchforwardfilter.h"
#include <QGraphicsObject>
#include <QGraphicsScene>

Q_DECLARE_OPERATORS_FOR_FLAGS(TouchForwardFilter::TouchPointConversionOptions)

const int TouchForwardFilter::MaxTouchPointId = 15;

TouchForwardFilter::TouchForwardFilter(QGraphicsObject *targetItem,
                                       ItemTouchState initialTargetTouchState,
                                       QGraphicsObject *eventOrigin,
                                       const QTouchEvent *initialTouchEvent)
    : QObject(targetItem), // Used for destroying filter when target is destroyed
      target(targetItem),
      isFirstOriginEvent(true),
      originTouchState(TouchInactive),
      targetTouchState(initialTargetTouchState)
{
    Q_ASSERT(targetItem && eventOrigin);

    connect(target.data(), SIGNAL(visibleChanged()),
            SLOT(deleteLaterIfTargetHidden()));

    // Install event filter to monitor touch events directly sent to target.
    target->installEventFilter(this);

    // Install filter for the main source of events to be forwarded.
    eventOrigin->installEventFilter(this);

    if (initialTouchEvent) {
        (void)handleTouchEventFromOrigin(*eventOrigin, *initialTouchEvent);
    }
}

TouchForwardFilter::~TouchForwardFilter()
{
    if (target) {
        target->removeEventFilter(this);
    }
}

bool TouchForwardFilter::eventFilter(QObject *watched, QEvent *event)
{
    bool eatEvent(false);

    if (event->type() == QEvent::TouchBegin
        || event->type() == QEvent::TouchUpdate
        || event->type() == QEvent::TouchEnd) {

        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);

        QGraphicsObject *watchedItem = qobject_cast<QGraphicsObject *>(watched);
        Q_ASSERT(watchedItem);

        if (watchedItem == target) {
            eatEvent = handleTouchEventFromTarget(*touchEvent);
        } else {
            eatEvent = handleTouchEventFromOrigin(*watchedItem, *touchEvent);
        }
    }

    return eatEvent;
}

bool TouchForwardFilter::handleTouchEventFromOrigin(const QGraphicsObject &originItem,
                                                    const QTouchEvent &touchEvent)
{
    // Forward event from origin to target. If this is the first event to forward
    // we need to ensure that all touch points get pressed before move or release.
    // If original event is already TouchBegin no extra event needs to be sent.

    if (!target) {
        // We're not handling origin events anymore, let it through.
        return false;
    }

    const QEvent::Type type = touchEvent.type();

    TouchPointConversionOptions conversionOptions = NoOption;

    if (isFirstOriginEvent) {
        isFirstOriginEvent = false;

        if (type != QEvent::TouchBegin) {
            sendEvent(convertTouchEvent(touchEvent,
                                        targetTouchState == TouchActive
                                        ? QEvent::TouchUpdate : QEvent::TouchBegin,
                                        originItem,
                                        ConvertToPress));

            // Discard last position for next touch points since we already
            // pressed them at current pos.
            conversionOptions = NoPress | DiscardLastPosition;
        }
    }

    QEvent::Type overriddenType = type;
    switch (type) {
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
        if (targetTouchState == TouchActive) {
            // Convert to TouchUpdate if target is already receiving touch events.
            overriddenType = QEvent::TouchUpdate;
        }
        break;
    case QEvent::TouchUpdate:
    default:
        break;
    }

    sendEvent(convertTouchEvent(touchEvent,
                                overriddenType,
                                originItem,
                                conversionOptions));

    if (type == QEvent::TouchEnd) {
        target = 0; // Stop forwarding events.
        deleteLater();
        isFirstOriginEvent = true;
        originTouchState = TouchInactive;
    } else {
        originTouchState = TouchActive;
    }
    return true;
}

bool TouchForwardFilter::handleTouchEventFromTarget(QTouchEvent &touchEvent)
{
    // Point here is to let the event to propagate but with modified type
    // ensuring that no multiple TouchBegin or TouchEnd events are delivered
    // to target.

    const QEvent::Type originalType = touchEvent.type();
    QEvent::Type overriddenType = originalType;

    switch (originalType) {
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
        if (originTouchState == TouchActive) {
            overriddenType = QEvent::TouchUpdate;
        }
        break;
    case QEvent::TouchUpdate:
    default:
        break;
    }

    if (overriddenType != originalType) {
        QTouchEvent overriddenTouchEvent(overriddenType);
        overriddenTouchEvent.setTouchPoints(touchEvent.touchPoints());
        overriddenTouchEvent.setTouchPointStates(touchEvent.touchPointStates());
        touchEvent = overriddenTouchEvent;
    }

    targetTouchState = originalType != QEvent::TouchEnd
                        ? TouchActive : TouchInactive;

    return false; // Let (modified) event propagate.
}

void TouchForwardFilter::sendEvent(const QTouchEvent &touchEvent)
{
    QTouchEvent event(touchEvent);
    target->removeEventFilter(this);
    target->scene()->sendEvent(target, &event);
    target->installEventFilter(this);
}

QTouchEvent TouchForwardFilter::convertTouchEvent(const QTouchEvent &touchEvent,
                                                  QEvent::Type overriddenType,
                                                  const QGraphicsObject &originItem,
                                                  TouchPointConversionOptions touchPointOptions) const
{
    QList<QTouchEvent::TouchPoint> touchPoints;
    foreach (const QTouchEvent::TouchPoint &touchPoint, touchEvent.touchPoints()) {
        touchPoints << convertTouchPoint(touchPoint, originItem, touchPointOptions);
    }

    QTouchEvent modifiedEvent(overriddenType);
    modifiedEvent.setTouchPoints(touchPoints);
    return modifiedEvent;
}

QTouchEvent::TouchPoint TouchForwardFilter::convertTouchPoint(const QTouchEvent::TouchPoint &touchPoint,
                                                              const QGraphicsObject &originItem,
                                                              TouchPointConversionOptions options) const
{
    QTouchEvent::TouchPoint tp(touchPoint);
    tp.setId(uniqueTouchId(originItem, tp.id()));
    tp.setPos(target->mapFromItem(&originItem, tp.pos()));
    tp.setScenePos(target->mapFromItem(&originItem, tp.scenePos()));
    tp.setScreenPos(target->mapFromItem(&originItem, tp.screenPos()));

    Qt::TouchPointState newState = tp.state();

    if (options & ConvertToPress && tp.state() != Qt::TouchPointPressed) {
        // Make it a press.
        newState = Qt::TouchPointPressed;

        // Press implies DiscardLastPosition.
        options |= DiscardLastPosition;
    } else if (options & NoPress && tp.state() == Qt::TouchPointPressed) {
        // Make press stationary
        newState = Qt::TouchPointStationary;

        // No last position for stationary.
        options |= DiscardLastPosition;
    }

    if (options & DiscardLastPosition && newState == Qt::TouchPointMoved) {
        newState = Qt::TouchPointStationary;
    }

    if (tp.state() != newState) {
        tp.setState(tp.isPrimary() ? (newState | Qt::TouchPointPrimary) : newState);
    }

    if (options & DiscardLastPosition) {
        tp.setLastPos(tp.pos());
        tp.setLastScenePos(tp.scenePos());
        tp.setLastScreenPos(tp.screenPos());
    } else {
        tp.setLastPos(target->mapFromItem(&originItem, tp.lastPos()));
        tp.setLastScenePos(target->mapFromItem(&originItem, tp.lastScenePos()));
        tp.setLastScreenPos(target->mapFromItem(&originItem, tp.lastScreenPos()));
    }
    return tp;
}

int TouchForwardFilter::uniqueTouchId(const QGraphicsObject &originItem, int id)
{
    // Point is to use originItem's address as a base for creating a unique id.

    // This to make sure it is really unique, even if we used pure
    // QGraphicsObjects reserved from adjacent memory blocks.
    Q_ASSERT(static_cast<unsigned int>(MaxTouchPointId) < sizeof(QGraphicsObject));

    if (id <= MaxTouchPointId) {
        id += reinterpret_cast<int>(&originItem);
    } // else already unique
    return id;
}

void TouchForwardFilter::deleteLaterIfTargetHidden()
{

    if (target && !target->isVisible()) {
        deleteLater();
    }
}
