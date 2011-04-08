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
