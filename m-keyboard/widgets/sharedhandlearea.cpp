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

#include "grip.h"
#include "handle.h"
#include "mimtoolbar.h"
#include "sharedhandlearea.h"

#include <QDebug>
#include <QGraphicsLinearLayout>


SharedHandleArea::SharedHandleArea(MImToolbar &toolbar, QGraphicsWidget *parent)
    : MWidget(parent),
      mainLayout(*new QGraphicsLinearLayout(Qt::Vertical, this)),
      invisibleHandle(*new Handle(this)),
      toolbarGrip(*new Grip(this)),
      zeroSizeToolbarGrip(*new QGraphicsWidget(this)),
      zeroSizeInvisibleHandle(*new QGraphicsWidget(this))
{
    mainLayout.setContentsMargins(0, 0, 0, 0);
    mainLayout.setSpacing(0);

    invisibleHandle.setObjectName("InvisibleHandle");
    invisibleHandle.hide();
    zeroSizeInvisibleHandle.setMaximumSize(0, 0);
    zeroSizeInvisibleHandle.show();
    mainLayout.addItem(&zeroSizeInvisibleHandle);
    connectHandle(invisibleHandle);

    toolbarGrip.setObjectName("KeyboardToolbarHandle");
    toolbarGrip.hide();
    zeroSizeToolbarGrip.setMaximumSize(0, 0);
    zeroSizeToolbarGrip.show();
    mainLayout.addItem(&zeroSizeToolbarGrip);
    connectHandle(toolbarGrip);

    connect(&toolbar, SIGNAL(availabilityChanged(bool)), this, SLOT(handleToolbarAvailability(bool)));

    Handle &toolbarHandle = *new Handle(this);
    toolbarHandle.setObjectName("KeyboardToolbarBackgroundHandle");
    toolbarHandle.setChild(&toolbar);
    connectHandle(toolbarHandle);

    mainLayout.addItem(&toolbarHandle);
    mainLayout.setAlignment(&toolbarHandle, Qt::AlignCenter);

    connect(&toolbar, SIGNAL(regionUpdated()), this, SIGNAL(regionUpdated()));
}


SharedHandleArea::~SharedHandleArea()
{
}


void SharedHandleArea::handleToolbarAvailability(const bool available)
{
    QGraphicsWidget &previousItem(*dynamic_cast<QGraphicsWidget *>(
                                      mainLayout.itemAt(ToolbarHandleIndex)));
    mainLayout.removeItem(&previousItem);
    previousItem.hide();
    QGraphicsWidget &newItem(available ? toolbarGrip : zeroSizeToolbarGrip);
    mainLayout.insertItem(ToolbarHandleIndex, &newItem);
    newItem.setVisible(available);
    emit regionUpdated();
}


void SharedHandleArea::connectHandle(const Handle &handle)
{
    connect(&handle, SIGNAL(flickLeft(const FlickGesture &)), this, SIGNAL(flickLeft(const FlickGesture &)));
    connect(&handle, SIGNAL(flickRight(const FlickGesture &)), this, SIGNAL(flickRight(const FlickGesture &)));
    connect(&handle, SIGNAL(flickUp(const FlickGesture &)), this, SIGNAL(flickUp(const FlickGesture &)));
    connect(&handle, SIGNAL(flickDown(const FlickGesture &)), this, SIGNAL(flickDown(const FlickGesture &)));
}


void SharedHandleArea::setInputMethodMode(const M::InputMethodMode mode)
{
    // Toggle invisible gesture handle area on/off
    QGraphicsWidget &previousItem(*dynamic_cast<QGraphicsWidget *>(
                                      mainLayout.itemAt(InvisibleHandleIndex)));
    mainLayout.removeItem(&previousItem);
    previousItem.hide();
    QGraphicsWidget &newItem(mode == M::InputMethodModeDirect ? invisibleHandle : zeroSizeInvisibleHandle);
    mainLayout.insertItem(InvisibleHandleIndex, &newItem);
    newItem.setVisible(mode == M::InputMethodModeDirect);
    emit regionUpdated();
}


QRegion SharedHandleArea::region(const bool includeExtraInteractiveAreas) const
{
    QRegion region(mapRectToScene(mainLayout.itemAt(ToolbarHandleIndex)->geometry()).toRect());

    region |= mapRectToScene(mainLayout.itemAt(ToolbarIndex)->geometry()).toRect();

    if (includeExtraInteractiveAreas) {
        region |= mapRectToScene(mainLayout.itemAt(InvisibleHandleIndex)->geometry()).toRect();
    }

    return region;
}
