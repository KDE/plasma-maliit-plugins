/* * This file is part of dui-keyboard *
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

#include <DuiTheme>

FlickUpButtonView::FlickUpButtonView(FlickUpButton *controller)
    : DuiButtonView(controller),
      controller(controller)
{
}


FlickUpButtonView::~FlickUpButtonView()
{
}


void FlickUpButtonView::applyStyle()
{
    QString name;

    DuiButtonView::applyStyle();

    name = style()->icon();

    // Set the icon, or clear if name not available.
    controller->setIconID(name);

    update();
}


void FlickUpButtonView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    DuiButtonView::mouseMoveEvent(event);

    if (controller->onMove(event->pos()))
        DuiButtonView::mouseReleaseEvent(event);
}

