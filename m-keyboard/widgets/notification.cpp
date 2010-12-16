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



#include "mvirtualkeyboardstyle.h"
#include "notification.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>

#include <MSceneManager>
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
    painter->setPen(border);
    painter->setBrush(background);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRoundedRect(rect(), 10, 10);
    painter->setPen(textColor);
    // Draw the normalized message
    painter->drawText(rect(), Qt::AlignCenter, message);
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
    const QFontMetrics fm(font);
    QStringList words;
    QString currentSection;
    int maxSectionWidth = -1;
    int sectionNum = 0;

    // Empty the normalized content
    message = "";
    // Split the message into words
    words = msg.split(' ');

    // Build the normalized text message
    currentSection = "";
    while (!words.isEmpty()) {
        currentSection += words.takeFirst()+' ';
        // The current string part with an additional word would exceed
        // the available width or there are no words left.
        if ((!words.isEmpty()
             && fm.width(currentSection+words[0]) + Margin * 2 >= area.width())
             || words.isEmpty()) {
            // Increase the section counter
            sectionNum++;
            // Remove the trailing space
            currentSection.remove(currentSection.size()-1, 1);
            // Find the maximum width of the sections
            if (fm.width(currentSection) > maxSectionWidth)
                maxSectionWidth = fm.width(currentSection);
            // Add the section to the normalized message text
            message += currentSection;
            // Add line break after the section if it is not the last yet
            if (!words.isEmpty())
                message += "\n";
            currentSection = "";
        }
    }

    // Reset the geometry
    const int width = maxSectionWidth + Margin * 2;
    const int height = fm.height()*sectionNum + Margin * 2;

    setGeometry(area.width() / 2 - width / 2,
                (area.height() - height) / 2,
                width, height);
}
