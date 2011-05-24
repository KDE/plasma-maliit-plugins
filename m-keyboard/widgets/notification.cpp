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



#include "mvirtualkeyboardstyle.h"
#include "notification.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>

#include <MSceneManager>
#include <MScalableImage>
#include <float.h>

namespace
{
    const int Margin = 10;           // Margin of the box surrounding text
    const int FrameCount = 15;       // How many frames to use in fading
    const int FadeTime = 300;        // Duration of fading in/out animation
    const int HoldTime = 700;        // Time to hold the widget in visible state
};


Notification::Notification(const MVirtualKeyboardStyleContainer *style, QGraphicsWidget *parent)
    : MWidget(parent),
      styleContainer(style),
      opacity(0)
{
    // Notification sets its own absolute opacity
    setFlag(ItemIgnoresParentOpacity, true);
    setZValue(FLT_MAX);

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
    painter->setRenderHint(QPainter::Antialiasing);

    const MScalableImage *backgroundImage = style()->notificationBackgroundImage();

    if (backgroundImage) {
        backgroundImage->draw(boundingRect().toRect(), painter);
    } else {
        painter->setPen(border);
        painter->setBrush(background);
        const int rounding = style()->notificationRounding();
        painter->drawRoundedRect(rect(), rounding, rounding);
    }

    painter->setPen(textColor);
    // Draw the normalized message
    QRectF textRect = rect();
    textRect.adjust(Margin, Margin, -Margin, -Margin);
    painter->drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, message);
}


void
Notification::displayText(const QString &msg, const QRectF &area)
{
    // Normalize the message. Use line breaks if the message
    // does not fit into the screen and set the geometry
    setMessageAndGeometry(msg, area);
    // Start to fade in
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
    font.setPixelSize(style()->notificationFontSize());
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

const MVirtualKeyboardStyleContainer &Notification::style() const
{
    return *styleContainer;
}

void Notification::setMessageAndGeometry(const QString &msg, const QRectF &area)
{
    message = msg;
    setGeometry(0, 0, area.width(), area.height());
}
