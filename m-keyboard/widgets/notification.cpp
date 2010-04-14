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



#include "duivirtualkeyboardstyle.h"
#include "notification.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>

#include <DuiSceneManager>
#include <duiplainwindow.h>

namespace
{
    const int Margin = 10;           // Margin of the box surrounding text
    const int FrameCount = 15;       // How many frames to use in fading
    const int FadeTime = 300;        // Duration of fading in/out animation
    const int HoldTime = 700;        // Time to hold the widget in visible state
};


Notification::Notification(DuiVirtualKeyboardStyleContainer *style, QGraphicsWidget *parent)
    : DuiWidget(parent),
      styleContainer(style)
{
    // Notification sets its own absolute opacity
    setFlag(ItemIgnoresParentOpacity, true);

    visibilityTimer.setInterval(HoldTime);
    visibilityTimer.setSingleShot(true);

    fadeTimeLine.setFrameRange(0, FrameCount);
    fadeTimeLine.setDuration(FadeTime);

    connect(&fadeTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(updateOpacity(int)));
    connect(&fadeTimeLine, SIGNAL(finished()),
            this, SLOT(fadingFinished()));

    connect(&visibilityTimer, SIGNAL(timeout()),
            this, SLOT(fadeOut()));

    getStyleValues();
    hide();
}


Notification::~Notification()
{
}


void
Notification::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setFont(font);
    painter->setPen(border);
    painter->setBrush(background);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRoundedRect(rect(), 10, 10);
    painter->setPen(textColor);
    painter->drawText(rect(), Qt::AlignCenter, message);
}


void
Notification::displayText(const QString &message)
{
    if (this->message != message) {
        this->message = message;
        resetGeometry();
    }

    fadeIn();
}


void
Notification::updateOpacity(int op)
{
    setOpacity(qreal(op) / qreal(FrameCount) * opacity);
    update();
}


void
Notification::fadingFinished()
{
    if (fadeTimeLine.direction() == QTimeLine::Forward) {
        // Notification is now shown an will be visible
        // as long as visibility timer runs.
        visibilityTimer.start();
    } else {
        // We have faded out
        hide();
    }
}


void Notification::fadeOut()
{
    fadeTimeLine.setDirection(QTimeLine::Backward);
    fadeTimeLine.start();
}


void Notification::getStyleValues()
{
    font = style()->notificationFont();
    border = style()->notificationBorderColor();
    background = style()->notificationBackgroundColor();
    textColor = style()->notificationTextColor();
    opacity = style()->notificationOpacity();
}


void Notification::fadeIn()
{
    // Handle fadeIn request in four different states:
    // not visible, fading in, visible, fading out

    if (fadeTimeLine.state() == QTimeLine::NotRunning) {
        if (isVisible()) {
            // Already visible so just prolong the visibility time
            // by restarting timer.
            visibilityTimer.start();
        } else {
            // The common case, begin fading in from hidden state.
            setOpacity(0.0);
            show();

            // Begin fading in
            fadeTimeLine.setDirection(QTimeLine::Forward);
            fadeTimeLine.start();
        }
    } else {
        // Fading in progress

        if (fadeTimeLine.direction() == QTimeLine::Forward) {
            // New fade in requested while fading in.
            // Do nothing, i.e. continue ongoing fading.
        } else {
            fadeTimeLine.toggleDirection();
        }
    }
}


void Notification::resetGeometry()
{
    const QFontMetrics fm(font);
    const int width = fm.width(message) + Margin * 2;
    const int height = fm.height() + Margin * 2;

    const QSize sceneSize = DuiPlainWindow::instance()->visibleSceneSize();

    setGeometry(sceneSize.width() / 2 - width / 2,
                -height,
                width, height);
}


const DuiVirtualKeyboardStyleContainer &Notification::style() const
{
    return *styleContainer;
}
