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
#include "panparameters.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QGraphicsSceneResizeEvent>

#include <MSceneManager>
#include <MScalableImage>
#include <float.h>

namespace
{
    const int FPS = 20; // Frames per second
}

Notification::Notification(QGraphicsItem *parent)
    : MStylableWidget(parent),
      textHorizontalAlignment(Qt::AlignHCenter),
      textVerticalAlignment(Qt::AlignVCenter),
      textWrap(false),
      mOpacity(0),
      frameCount(1),
      textLayout(new QStaticText()),
      maximumTextWidth(0),
      dirty(true),
      scale(1.0f)
{
    // Notification sets its own absolute opacity
    setFlag(ItemIgnoresParentOpacity, true);
    setZValue(FLT_MAX);

    visibilityTimer.setSingleShot(true);

    connect(&fadeTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(updateOpacityByFrame(int)));
    connect(&fadeTimeLine, SIGNAL(finished()),
            this, SLOT(fadingFinished()));

    connect(&visibilityTimer, SIGNAL(timeout()),
            this, SLOT(fadeOut()));

    hide();
}


Notification::~Notification()
{
    connectPanParameters(0);
}


void
Notification::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setFont(font);
    painter->setRenderHint(QPainter::Antialiasing);

    const MScalableImage *backgroundImage = style()->backgroundImage();

    const qreal oldOpacity = painter->opacity();
    painter->setOpacity(oldOpacity * style()->backgroundOpacity());
    if (backgroundImage) {
        backgroundImage->draw(boundingRect().toRect(), painter);
    } else {
        painter->setPen(border);
        painter->setBrush(background);
        const int rounding = style()->rounding();
        painter->drawRoundedRect(rect(), rounding, rounding);
    }
    painter->setOpacity(oldOpacity);

    painter->setPen(textColor);
    // Draw the normalized message
    QRectF textRect = rect();

    QPointF textOffset;

    textOffset.setX((textRect.width() / scale - textLayout->size().width()) / 2);

    if (textVerticalAlignment == Qt::AlignVCenter) {
        textOffset.setY((textRect.height()
                         - style()->paddingBottom()
                         + style()->paddingTop()
                         - textLayout->size().height()) / (scale * 2.0f));
    } else if (textVerticalAlignment == Qt::AlignBottom) {
        textOffset.setY((textRect.height() - style()->paddingBottom()) / scale
                         - textLayout->size().height() / (scale / 2.0f + 0.5f));
    }

    const QTransform oldTransform = painter->transform();
    painter->setTransform(transform, true);
    painter->drawStaticText(textOffset, *textLayout);
    painter->setTransform(oldTransform, false);
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

void Notification::setText(const QString &text)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << text;
    message = text;
    dirty = true;
    reLayout();
}

QString Notification::text() const
{
    return message;
}

void Notification::applyStyle()
{
    font = style()->font();
    font.setPixelSize(style()->fontSize());
    border = style()->borderColor();
    background = style()->backgroundColor();
    textColor = style()->textColor();
    textHorizontalAlignment = style()->textHorizontalAlignment();
    textVerticalAlignment = style()->textVerticalAlignment();
    textWrap = style()->textWrap();
    mOpacity = style()->opacity();
    fadeTimeLine.setDuration(style()->fadeTime());
    visibilityTimer.setInterval(style()->holdTime());
    frameCount = style()->fadeTime() / FPS;
    fadeTimeLine.setFrameRange(0, frameCount);
}


void
Notification::updateOpacityByFrame(int frameNumber)
{
    setOpacity(qreal(frameNumber) / qreal(frameCount) * mOpacity);
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

void Notification::setMessageAndGeometry(const QString &msg, const QRectF &area)
{
    message = msg;
    setPos(area.topLeft());
    // setMaximumTextWidth() will change the dirty flag to true.
    setMaximumTextWidth(area.width());
}

void Notification::updatePos(const QPointF &pos)
{
    setPos(pos);
    update();
}

void Notification::updateOpacity(qreal opacity)
{
    setOpacity(opacity);
}

void Notification::updateScale(qreal scale)
{
    setScale(scale);
}

void Notification::reLayout()
{
    if (!dirty)
        return;
    dirty = false;
    delete textLayout;
    textLayout = new QStaticText();
    QTextOption textOption;
    textLayout->setPerformanceHint(QStaticText::AggressiveCaching);
    textOption.setAlignment(textHorizontalAlignment | textVerticalAlignment);
    if (textWrap) {
        textOption.setWrapMode(QTextOption::WordWrap);
    } else {
        textOption.setWrapMode(QTextOption::NoWrap);
    }
    textLayout->setTextOption(textOption);
    textLayout->setText(message);
    textLayout->prepare(QTransform(), font);
    if (textWrap
        && maximumTextWidth > 0
        && maximumTextWidth < textLayout->size().width()) {
        textLayout->setTextWidth(maximumTextWidth - style()->paddingLeft()
                                 - style()->paddingRight());
        textLayout->prepare(QTransform(), font);
    }

    if (style()->textMaximumWidth() > 0
        && style()->textMaximumWidth() < textLayout->size().width()) {
        scale = qreal(style()->textMaximumWidth()) / textLayout->size().width();
        transform = QTransform::fromScale(scale, scale);
        textLayout->prepare(transform, font);
    } else {
        scale = 1.0f;
        transform = QTransform();
    }
    updateGeometry();
}

void Notification::setMaximumTextWidth(qreal textWidth)
{
    maximumTextWidth = textWidth;
    dirty = true;
    reLayout();
}

QSizeF Notification::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    switch (which) {
    case Qt::MinimumSize: {
        return MStylableWidget::sizeHint(which, constraint);
    }
    case Qt::MaximumSize: {
            return QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    }
    case Qt::PreferredSize: {
        QSizeF size = textLayout->size();
        size += QSize(style()->paddingLeft() + style()->paddingRight(),
                      style()->paddingTop() + style()->paddingBottom());
        size.boundedTo(constraint);
        QSizeF parentSize = style()->preferredSize();
        if (parentSize.width() < 0) {
            parentSize.setWidth(size.width());
        }
        if (parentSize.height() < 0) {
            parentSize.setHeight(size.height());
        }
        return parentSize;
    }
    default:
        qWarning() << __PRETTY_FUNCTION__
            << "don't know how to handle the value of 'which':" << which;
    }
    return QSizeF(0, 0);
}

void Notification::connectPanParameters(PanParameters *parameters)
{
    if (!mParameters.isNull()) {
        disconnect(mParameters.data(), 0, this, 0);
    }
    mParameters = parameters;

    if (!mParameters.isNull())  {
        connect(parameters, SIGNAL(positionChanged(QPointF)),
                this,       SLOT(updatePos(QPointF)));

        connect(parameters, SIGNAL(opacityChanged(qreal)),
                this,       SLOT(updateOpacity(qreal)));

        connect(parameters, SIGNAL(scaleChanged(qreal)),
                this,       SLOT(updateScale(qreal)));
    }
}
