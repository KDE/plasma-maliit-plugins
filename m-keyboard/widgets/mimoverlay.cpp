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



#include "mimoverlay.h"

#include <MSceneManager>
#include <MGConfItem>
#include <mplainwindow.h>

#include <QString>
#include <float.h>

namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSetting = "/meegotouch/inputmethods/multitouch/enabled";
};

MImOverlay::MImOverlay()
    : MSceneWindow()
{
    setManagedManually(true);
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(this);
    // The z-value should always be more than vkb and text widget's z-value
    setZValue(FLT_MAX);

    // By default multi-touch is disabled
    setAcceptTouchEvents(MGConfItem(MultitouchSetting).value().toBool());

    setGeometry(QRectF(QPointF(0, 0), MPlainWindow::instance()->sceneManager()->visibleSceneSize()));

    connect(MPlainWindow::instance()->sceneManager(), SIGNAL(orientationChanged(M::Orientation)),
            this, SLOT(handleOrientationChanged()));
    hide();
}

MImOverlay::~MImOverlay()
{
}

bool MImOverlay::sceneEvent(QEvent *e)
{
    MWidget::sceneEvent(e);

    // eat all the touch and mouse press/release  events to avoid these events
    // go to the background virtual keyboard.
    e->setAccepted(e->isAccepted()
                   || e->type() == QEvent::TouchBegin
                   || e->type() == QEvent::TouchUpdate
                   || e->type() == QEvent::TouchEnd
                   || e->type() == QEvent::GraphicsSceneMousePress
                   || e->type() == QEvent::GraphicsSceneMouseRelease);
    return e->isAccepted();
}

void MImOverlay::handleOrientationChanged()
{
    setGeometry(QRectF(QPointF(0, 0), MPlainWindow::instance()->sceneManager()->visibleSceneSize()));
}
