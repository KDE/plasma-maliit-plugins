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

    static const FlickEmitter emitters[4] = { &Handle::flickLeft, &Handle::flickRight,
                                              &Handle::flickUp, &Handle::flickDown };
    (this->*emitters[static_cast<int>(gesture.direction())])(gesture);
}
