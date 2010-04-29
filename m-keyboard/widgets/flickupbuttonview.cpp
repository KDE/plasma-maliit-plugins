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



#include "flickupbuttonview.h"
#include "flickupbutton.h"

#include <QGraphicsSceneMouseEvent>

#include <MTheme>

FlickUpButtonView::FlickUpButtonView(FlickUpButton *controller)
    : MButtonView(controller),
      controller(controller)
{
}


FlickUpButtonView::~FlickUpButtonView()
{
}


void FlickUpButtonView::applyStyle()
{
    QString name;

    MButtonView::applyStyle();

    name = style()->icon();

    // Set the icon, or clear if name not available.
    controller->setIconID(name);

    update();
}


void FlickUpButtonView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    MButtonView::mouseMoveEvent(event);

    if (controller->onMove(event->pos()))
        MButtonView::mouseReleaseEvent(event);
}

