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

#include "borderpanrecognizer.h"
#include "pangesture.h"

#include <QGraphicsWidget>
#include <QGraphicsSceneMouseEvent>
#include <QPoint>
#include <QDebug>

namespace {
    const int DefaultPanGestureTimeout = 250;
    const int DefaultStartThreshold = 20;
    const int DefaultFinishThreshold = 200;
    const int DefaultInitialMovement = 20;
}

Qt::GestureType BorderPanRecognizer::sharedType = static_cast<Qt::GestureType>(0);
BorderPanRecognizer *BorderPanRecognizer::sharedInstance = 0;

BorderPanRecognizer::BorderPanRecognizer() :
    QGestureRecognizer()
    , timeoutInterval(DefaultPanGestureTimeout)
    , startThreshold(DefaultStartThreshold)
    , finishThreshold(DefaultFinishThreshold)
    , initialMovement(DefaultInitialMovement)
{
}

BorderPanRecognizer::~BorderPanRecognizer()
{
}

BorderPanRecognizer *BorderPanRecognizer::instance()
{
    return sharedInstance;
}

void BorderPanRecognizer::registerSharedRecognizer()
{
    if (!sharedInstance) {
        sharedInstance = new BorderPanRecognizer;

        // Ownership of recognizer is transferred to the application.
        sharedType = registerRecognizer(sharedInstance);
    }
}

Qt::GestureType BorderPanRecognizer::sharedGestureType()
{
    return sharedType;
}

void BorderPanRecognizer::unregisterSharedRecognizer()
{
    if (sharedInstance) {
        unregisterRecognizer(sharedType);
        sharedType = static_cast<Qt::GestureType>(0);

        // instance owned by application
        sharedInstance = 0;
    }
}


QGesture *BorderPanRecognizer::create(QObject * /*target*/)
{
    return new PanGesture;
}

QGestureRecognizer::Result BorderPanRecognizer::recognize(QGesture * gesture,
                                                          QObject * watched,
                                                          QEvent * event)
{
    Result result = CancelGesture;

    if (!gesture || !event) {
        return result;
    }

    // We know we're dealing with PanGesture.
    PanGesture &panGesture = static_cast<PanGesture &>(*gesture);

    // We only handle GraphicsSceneMouseEvent events.
    const QGraphicsSceneMouseEvent *mouseEvent(static_cast<const QGraphicsSceneMouseEvent *>(event));

    // We are using this recognizer for QGraphicsWidgets only
    const QGraphicsWidget *widget(static_cast<const QGraphicsWidget *>(watched));
    Q_ASSERT(widget);

    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        result = recognizeInit(panGesture, mouseEvent,
                               widget);
        break;

    case QEvent::GraphicsSceneMouseMove:
        if (panGesture.pressReceived) {
            result = recognizeUpdate(panGesture, mouseEvent,
                                     widget);
        }
        break;

    case QEvent::GraphicsSceneMouseRelease:
        result = recognizeFinalize(panGesture, mouseEvent, widget);
        break;

    default:
        result = Ignore;
        break;
    }

    if (event->type() == QEvent::UngrabMouse
        && !panGesture.cancelled
        && (gesture->state() == Qt::GestureStarted
            || gesture->state() == Qt::GestureUpdated)) {
        result = CancelGesture;
    }

    return result;
}

void BorderPanRecognizer::reset(QGesture *gesture)
{
    PanGesture &panGesture = static_cast<PanGesture &>(*gesture);
    panGesture.reset();
    QGestureRecognizer::reset(gesture);
}

QGestureRecognizer::Result
BorderPanRecognizer::recognizeInit(PanGesture &gesture,
                                   const QGraphicsSceneMouseEvent *mouseEvent,
                                   const QGraphicsWidget * widget)
{
    gesture.sceneStartPos = mouseEvent->scenePos().toPoint();
    gesture.startPos = mouseEvent->pos().toPoint();
    const int x = mouseEvent->pos().toPoint().x();

    if (x > startThreshold
        && x < (widget->size().width() - startThreshold)) {
        gesture.cancelled = true;
        return CancelGesture;
    }

    gesture.m_direction = (x <= startThreshold) ? PanGesture::PanRight : PanGesture::PanLeft;

    if(gesture.state() != Qt::NoGesture) {
        reset(&gesture);
    }

    gesture.setHotSpot(mouseEvent->screenPos());
    gesture.pressReceived = true;
    gesture.cancelled = false;
    gesture.currentPos = mouseEvent->pos().toPoint();
    gesture.startTimer.setInterval(timeoutInterval);
    gesture.startTimer.start();

    return MayBeGesture;
}

QGestureRecognizer::Result
BorderPanRecognizer::recognizeUpdate(PanGesture &gesture,
                                     const QGraphicsSceneMouseEvent *mouseEvent,
                                     const QGraphicsWidget * /*widget*/)
{
    Result result = TriggerGesture;

    gesture.setHotSpot(mouseEvent->screenPos());
    gesture.currentPos = mouseEvent->pos().toPoint();

    if (gesture.triggered) {
        return result;
    }

    if (gesture.cancelled) {
        return Ignore;
    }

    //const int center = widget->size().width() / 2;
    //const int x = mouseEvent->pos().toPoint().x();

    if (gesture.cancelled
        /*|| abs(center - gesture.startPos.x()) < abs(center - x)*/) {
        result = CancelGesture;
        gesture.cancelled = true;
        gesture.triggered = false;
    } else if (qAbs(gesture.distance().x()) < initialMovement) {
        result = Ignore;
    } else {
        gesture.startTimer.stop();
        gesture.cancelled = false;
        gesture.triggered = true;
    }

    return result;
}

QGestureRecognizer::Result
BorderPanRecognizer::recognizeFinalize(PanGesture &gesture,
                                       const QGraphicsSceneMouseEvent *mouseEvent,
                                       const QGraphicsWidget * /*widget*/)
{
    Result result = CancelGesture;
    gesture.currentPos = mouseEvent->pos().toPoint();

    if (!gesture.cancelled && gesture.triggered && abs(gesture.distance().x()) > finishThreshold) {
        result = FinishGesture;
    } else {
        result = CancelGesture;
    }

    return result;
}

void BorderPanRecognizer::setTimeout(int timeoutInterval)
{
    this->timeoutInterval = timeoutInterval;
}

void BorderPanRecognizer::setStartThreshold(int threshold)
{
    startThreshold = threshold;
}

void BorderPanRecognizer::setFinishThreshold(int threshold)
{
    finishThreshold = threshold;
}

void BorderPanRecognizer::setInitialMovement(int initialMovement)
{
    this->initialMovement = initialMovement;
}
