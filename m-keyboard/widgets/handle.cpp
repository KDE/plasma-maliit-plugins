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

#include "handle.h"
#include "flickgesture.h"
#include "flickgesturerecognizer.h"

#include <QDebug>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>

Handle::Handle(QGraphicsWidget *parent)
    : MStylableWidget(parent),
      mainLayout(*new QGraphicsLinearLayout(Qt::Vertical, this))
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    mainLayout.setContentsMargins(0, 0, 0, 0);

    grabGesture(FlickGestureRecognizer::sharedGestureType());
}

Handle::~Handle()
{
}

bool Handle::event(QEvent *e)
{
    bool eaten = false;

    if (e->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(e);
        const Qt::GestureType flickGestureType = FlickGestureRecognizer::sharedGestureType();
        FlickGesture *flick = static_cast<FlickGesture *>(gestureEvent->gesture(flickGestureType));

        if (flick) {
            eaten = true;
            if (flick->state() == Qt::GestureStarted) {
                e->accept();
            } else if (flick->state() == Qt::GestureFinished) {
                flickGestureEvent(*flick);
            }
        }
    }

    return eaten || MStylableWidget::event(e);
}

void Handle::setChild(QGraphicsLayoutItem *widget)
{
    if (mainLayout.itemAt(0)) {
        mainLayout.removeAt(0);
    }
    mainLayout.insertItem(0, widget);
}

void Handle::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    // Do nothing except implicit mouse grab for flick gesture to work.
}

typedef void (Handle::*FlickEmitter)(const FlickGesture &gesture);

void Handle::flickGestureEvent(FlickGesture &gesture)
{
    if (gesture.direction() == FlickGesture::NoDirection) {
        return;
    }

    static const FlickEmitter emitters[2] = { &Handle::flickUp, &Handle::flickDown };
    (this->*emitters[static_cast<int>(gesture.direction())])(gesture);
}
