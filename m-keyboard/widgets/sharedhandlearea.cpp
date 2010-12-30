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
#include "mimtoolbar.h"
#include "sharedhandlearea.h"

#include <mplainwindow.h>

#include <QDebug>
#include <QGraphicsLinearLayout>

#include <limits>


SharedHandleArea::SharedHandleArea(MImToolbar &toolbar, QGraphicsWidget *parent)
    : MWidget(parent),
      mainLayout(*new QGraphicsLinearLayout(Qt::Vertical, this)),
      invisibleHandle(*new Handle(this)),
      zeroSizeInvisibleHandle(*new QGraphicsWidget(this)),
      toolbar(toolbar),
      inputMethodMode(M::InputMethodModeNormal)
{
    zeroSizeInvisibleHandle.setObjectName("zeroSizeInvisibleHandle");

    mainLayout.setContentsMargins(0, 0, 0, 0);
    mainLayout.setSpacing(0);

    invisibleHandle.setObjectName("InvisibleHandle");
    invisibleHandle.hide();
    zeroSizeInvisibleHandle.setMaximumSize(0, 0);
    zeroSizeInvisibleHandle.show();
    mainLayout.addItem(&zeroSizeInvisibleHandle);
    connectHandle(invisibleHandle);

    mainLayout.addItem(&toolbar);
    mainLayout.setAlignment(&toolbar, Qt::AlignCenter);

    connect(&toolbar, SIGNAL(regionUpdated()), this, SLOT(updatePositionAndRegion()));
    connect(this, SIGNAL(visibleChanged()), this, SLOT(updatePositionAndRegion()));
}


SharedHandleArea::~SharedHandleArea()
{
}


void SharedHandleArea::connectHandle(const Handle &handle)
{
    connect(&handle, SIGNAL(flickLeft(const FlickGesture &)), this, SIGNAL(flickLeft(const FlickGesture &)));
    connect(&handle, SIGNAL(flickRight(const FlickGesture &)), this, SIGNAL(flickRight(const FlickGesture &)));
    connect(&handle, SIGNAL(flickUp(const FlickGesture &)), this, SIGNAL(flickUp(const FlickGesture &)));
    connect(&handle, SIGNAL(flickDown(const FlickGesture &)), this, SIGNAL(flickDown(const FlickGesture &)));
}


void SharedHandleArea::updateInvisibleHandleVisibility()
{
    // For now we never enable the invisible handle.  The code is kept here until it's
    // certain the invisible handle won't be needed anymore.  Note: if you enable this,
    // you probably need to connect flick signals of this class and enable corresponding
    // ut_mkeyboardhost region test code.  If/When you remove this, you can also remove
    // setInputMethodMode, handleToolbarTypeChange and typeChanged signal from MImToolbar,
    // connectHandle and in fact everything related to handles from this file and the
    // header.  You also need to update addRegion and rename this class to something more
    // meaningful.
#if 0
    // Toggle invisible gesture handle area on/off
    const bool showInvisibleHandle(inputMethodMode == M::InputMethodModeDirect
                                   && !standardToolbar);
    QGraphicsWidget &previousItem(*dynamic_cast<QGraphicsWidget *>(
                                      mainLayout.itemAt(InvisibleHandleIndex)));
    mainLayout.removeItem(&previousItem);
    previousItem.hide();
    QGraphicsWidget &newItem(showInvisibleHandle ? invisibleHandle : zeroSizeInvisibleHandle);
    mainLayout.insertItem(InvisibleHandleIndex, &newItem);
    newItem.setVisible(showInvisibleHandle);
    updatePositionAndRegion(SignalsEnforce);
#endif
}

void SharedHandleArea::setInputMethodMode(const M::InputMethodMode mode)
{
    inputMethodMode = mode;
    updateInvisibleHandleVisibility();
}


QRegion SharedHandleArea::addRegion(const QRegion &region,
                                    bool includeExtraInteractiveAreas) const
{
    QRegion result = region;

    result |= toolbar.region();
    if (includeExtraInteractiveAreas) {
        result |= QRegion(mapRectToScene(mainLayout.itemAt(InvisibleHandleIndex)->geometry()).toRect());
    }

    return result;
}

void SharedHandleArea::updatePositionAndRegion(SignalsMode signalsMode)
{
    QPointF position = pos();

    layout()->invalidate();
    layout()->activate();
    resize(geometry().width(), layout()->preferredHeight());

    updatePosition();

    bool emitSignals = false;

    switch (signalsMode) {
    case SignalsBlock:
        emitSignals = false;
        break;
    case SignalsEnforce:
        emitSignals = true;
        break;
    case SignalsAuto:
        emitSignals = (position != pos());
        break;
    }

    if (emitSignals) {
        emit regionUpdated();
        emit inputMethodAreaUpdated();
    }
}

void SharedHandleArea::updatePosition()
{
    qreal bottom(std::numeric_limits<qreal>::max());
    bool widgetVisible(false);

    foreach (const QGraphicsWidget *widget, watchedWidgets) {
        if (widget && widget->isVisible()) {
            bottom = qMin(widget->pos().y(), bottom);
            widgetVisible = true;
        }
    }

    const QPointF newPos(0, bottom - size().height());

    if (widgetVisible && (newPos != pos())) {
        setPos(newPos);
    }
}

void SharedHandleArea::watchOnWidget(QGraphicsWidget *widget)
{
    if (!widget) {
        return;
    }

    connect(widget, SIGNAL(yChanged()), this, SLOT(updatePosition()));
    connect(widget, SIGNAL(visibleChanged()), this, SLOT(updatePosition()));
    watchedWidgets.append(widget);
    updatePositionAndRegion(SignalsAuto);

    watchedWidgets.removeAll(QPointer<QGraphicsWidget>()); //remove all invalid pointers
}

void SharedHandleArea::finalizeOrientationChange()
{
    //set proper width
    resize(MPlainWindow::instance()->visibleSceneSize().width(),
           size().height());
    updatePositionAndRegion(SharedHandleArea::SignalsBlock);
}

