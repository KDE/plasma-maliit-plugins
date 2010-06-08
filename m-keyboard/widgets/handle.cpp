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
#include <QGraphicsSceneMouseEvent>
#include "flickgesture.h"

#include <QDebug>
#include <QGraphicsLinearLayout>

Handle::Handle(QGraphicsWidget *parent)
    : MStylableWidget(parent),
      mainLayout(*new QGraphicsLinearLayout(Qt::Vertical, this))
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    mainLayout.setContentsMargins(0, 0, 0, 0);
}

Handle::~Handle()
{
}

bool Handle::event(QEvent *e)
{
    const QGraphicsSceneMouseEvent *ev(dynamic_cast<const QGraphicsSceneMouseEvent *>(e));

    if (e->type() == QEvent::GraphicsSceneMousePress) {
        e->setAccepted(true);
        startPosition = ev->pos();
        return true;
    } else if (e->type() == QEvent::GraphicsSceneMouseRelease) {
        e->setAccepted(true);

        const qreal MovementTresholdX(20);
        const qreal MovementTresholdY(20);
        const QPointF diff(ev->pos() - startPosition);

        FlickGesture gesture;
        gesture.setPositionDifference(diff);

        if ((abs(diff.x()) > MovementTresholdX)
            && (abs(diff.x()) > abs(diff.y()))) {
            gesture.setDirection(diff.x() > 0 ? FlickGesture::Right : FlickGesture::Left);
            flickGestureEvent(gesture);
        } else if ((abs(diff.y()) > MovementTresholdY)) {
            gesture.setDirection(diff.y() > 0 ? FlickGesture::Down : FlickGesture::Up);
            flickGestureEvent(gesture);
        }

        return true;
    }
    return MStylableWidget::event(e);
}

void Handle::setChild(QGraphicsLayoutItem *widget)
{
    if (mainLayout.itemAt(0)) {
        mainLayout.removeAt(0);
    }
    mainLayout.insertItem(0, widget);
}

typedef void (Handle::*FlickEmitter)(const FlickGesture &gesture);

void Handle::flickGestureEvent(FlickGesture &gesture)
{
    static const FlickEmitter emitters[4] = { &Handle::flickLeft, &Handle::flickRight,
                                              &Handle::flickUp, &Handle::flickDown };
    (this->*emitters[static_cast<int>(gesture.direction())])(gesture);
}
