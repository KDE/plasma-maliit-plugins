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
#include "notificationarea.h"
#include "notificationpanparameters.h"
#include "mplainwindow.h"
#include "notification.h"

#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>

#include <MSceneManager>
#include <MScalableImage>
#include <float.h>


NotificationArea::NotificationArea(QGraphicsItem *parent)
    : MStylableWidget(parent),
      outgoingNotification(new Notification(this)),
      incomingNotification(new Notification(this)),
      assistantNotification(new Notification(this)),
      outgoingNotificationParameters(new NotificationPanParameters(this)),
      incomingNotificationParameters(new NotificationPanParameters(this)),
      assistantNotificationParameters(new NotificationPanParameters(this)),
      hideAnimationGroup(this),
      showAnimation(this, "opacity")
{
    outgoingNotification->connectPanParameters(outgoingNotificationParameters);
    incomingNotification->connectPanParameters(incomingNotificationParameters);
    assistantNotification->connectPanParameters(assistantNotificationParameters);

    connect(&hideAnimationGroup, SIGNAL(finished()),
            this, SLOT(onNotificationAnimationFinished()));

    connect(&hideAnimationGroup, SIGNAL(finished()),
            this, SLOT(reset()));

    setVisible(false);
}


NotificationArea::~NotificationArea()
{
}

void NotificationArea::prepareNotifications(PanGesture::PanDirection direction)
{
    qDebug() << __PRETTY_FUNCTION__;

    const int screenWidth
        = (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait)
          ? MPlainWindow::instance()->visibleSceneSize(M::Landscape).height()
          : MPlainWindow::instance()->visibleSceneSize(M::Landscape).width();

    // prepare notifications
    qreal notificationMaximumWidth
        = style()->notificationMaximumWidth();

    outgoingNotification->setMaximumTextWidth(notificationMaximumWidth);
    incomingNotification->setMaximumTextWidth(notificationMaximumWidth);
    assistantNotification->setMaximumTextWidth(notificationMaximumWidth);

    qreal outgoingNotificationFromScale
        = qBound<qreal>(0, qreal(notificationMaximumWidth / outgoingNotification->preferredWidth()), 1.0);

    qreal outgoingNotificationToScale
        = style()->smallSizeScaleFactor();

    QPointF outgoingNotificationFromPos
        = QPointF((screenWidth - outgoingNotification->preferredWidth()
                   * outgoingNotificationFromScale) / 2,
                  preferredHeight()
                  - outgoingNotification->preferredHeight()
                  * outgoingNotificationFromScale);

    qreal outgoingNotificationToPosX
        = (direction == PanGesture::PanRight)
          ? screenWidth
          : - outgoingNotification->preferredWidth()
            * outgoingNotificationToScale;

    qreal outgoingNotificationToPosY
        = preferredHeight()
          - outgoingNotification->preferredHeight() * outgoingNotificationToScale;

    QPointF outgoingNotificationToPos
        = QPointF(outgoingNotificationToPosX, outgoingNotificationToPosY);

    outgoingNotification->updateScale(outgoingNotificationFromScale);
    outgoingNotification->updateOpacity(1.0);
    outgoingNotification->updatePos(outgoingNotificationFromPos);
    outgoingNotification->setVisible(false);
    outgoingNotificationParameters->setPositionRange(
        outgoingNotificationFromPos, outgoingNotificationToPos);
    outgoingNotificationParameters->setScaleRange(outgoingNotificationFromScale,
                                                  outgoingNotificationToScale);
    outgoingNotificationParameters->setOpacityRange(1.0, 0.0);
    outgoingNotificationParameters->setPositionProgressRange(
            style()->positionStartProgress(),
            style()->positionEndProgress());
    outgoingNotificationParameters->setScaleProgressRange(
            style()->scaleStartProgress(),
            style()->scaleEndProgress());
    outgoingNotificationParameters->setOpacityProgressRange(
            style()->opacityStartProgress(),
            style()->opacityEndProgress());

    qreal leftAndRightSpace
        = (outgoingNotification->preferredWidth()
           < screenWidth * (1 - style()->initialEdgeMaximumVisibleWidth() * 2))
          ? screenWidth * style()->initialEdgeMaximumVisibleWidth()
          : (screenWidth - notificationMaximumWidth) / 2;
    // prepare left and right notifications
    if (direction == PanGesture::PanRight) {
        incomingNotification->setText(mLeftLayoutTitle);
        assistantNotification->setText(mRightLayoutTitle);
    } else {
        incomingNotification->setText(mRightLayoutTitle);
        assistantNotification->setText(mLeftLayoutTitle);
    }

    qreal incomingNotificationFromScale = outgoingNotificationToScale;

    qreal incomingNotificationToScale
        = qBound<qreal>(0, qreal(notificationMaximumWidth / incomingNotification->preferredWidth()), 1.0);

    qreal incomingNotificationFromPosX = 0;
    if (incomingNotification->preferredWidth() * incomingNotificationFromScale
        > leftAndRightSpace) {

       incomingNotificationFromPosX
        = (direction == PanGesture::PanRight)
          ? leftAndRightSpace
            - incomingNotification->preferredWidth() * incomingNotificationFromScale
          : screenWidth - leftAndRightSpace;
    } else {

       incomingNotificationFromPosX
        = (direction == PanGesture::PanRight)
          ? 0
          : screenWidth - incomingNotification->preferredWidth() * incomingNotificationFromScale;
    }

    qreal incomingNotificationFromPosY
        = preferredHeight()
          - incomingNotification->preferredHeight() * incomingNotificationFromScale;
    QPointF incomingNotificationFromPos
        = QPointF(incomingNotificationFromPosX, incomingNotificationFromPosY);

    QPointF incomingNotificationToPos
        = QPointF((screenWidth - incomingNotification->preferredWidth()
                   * incomingNotificationToScale) / 2,
                  preferredHeight()
                  - incomingNotification->preferredHeight()
                  * incomingNotificationToScale);

    incomingNotification->updateScale(incomingNotificationFromScale);
    incomingNotification->updatePos(incomingNotificationFromPos);
    incomingNotification->updateOpacity(style()->initialEdgeOpacity());
    incomingNotification->setVisible(false);
    incomingNotificationParameters->setPositionRange(
        incomingNotificationFromPos, incomingNotificationToPos);
    incomingNotificationParameters->setScaleRange(incomingNotificationFromScale,
                                                  incomingNotificationToScale);
    incomingNotificationParameters->setOpacityRange(style()->initialEdgeOpacity(),
                                                    1.0);

    incomingNotificationParameters->setPositionProgressRange(
            style()->positionStartProgress(),
            style()->positionEndProgress());
    incomingNotificationParameters->setScaleProgressRange(
            style()->scaleStartProgress(),
            style()->scaleEndProgress());
    incomingNotificationParameters->setOpacityProgressRange(
            style()->opacityStartProgress(),
            style()->opacityEndProgress());


    qreal assistantNotificationFromScale = outgoingNotificationToScale;

    qreal assistantNotificationFromPosX = 0;

    if (assistantNotification->preferredWidth() * assistantNotificationFromScale
        > leftAndRightSpace) {

       assistantNotificationFromPosX
        = (direction == PanGesture::PanRight)
          ? screenWidth - leftAndRightSpace
          : leftAndRightSpace
            - assistantNotification->preferredWidth() * assistantNotificationFromScale;
    } else {

       assistantNotificationFromPosX
        = (direction == PanGesture::PanRight)
          ? screenWidth - assistantNotification->preferredWidth() * assistantNotificationFromScale
          : 0;
    }

    qreal assistantNotificationFromPosY
        = preferredHeight()
          - assistantNotification->preferredHeight() * assistantNotificationFromScale;
    qreal assistantNotificationToPosY
        = preferredHeight()
          - assistantNotification->preferredHeight() * outgoingNotificationToScale;

    QPointF assistantNotificationFromPos
        = QPointF(assistantNotificationFromPosX, assistantNotificationFromPosY);

    qreal outgoingNotificationXMoveMent
        = outgoingNotificationToPos.x() - outgoingNotificationFromPos.x();
    QPointF assistantNotificationToPos
        = QPointF(assistantNotificationFromPos.x() + outgoingNotificationXMoveMent,
                  assistantNotificationToPosY);

    assistantNotification->updateScale(assistantNotificationFromScale);
    assistantNotification->updateOpacity(style()->initialEdgeOpacity());
    assistantNotification->updatePos(assistantNotificationFromPos);
    assistantNotification->setVisible(false);
    assistantNotificationParameters->setPositionRange(
        assistantNotificationFromPos, assistantNotificationToPos);
    assistantNotificationParameters->setScaleRange(assistantNotificationFromScale,
                                               assistantNotificationFromScale * outgoingNotificationToScale);
    assistantNotificationParameters->setOpacityRange(style()->initialEdgeOpacity(),
                                                 0.0);

    assistantNotificationParameters->setPositionProgressRange(
            style()->positionStartProgress(),
            style()->positionEndProgress());
    assistantNotificationParameters->setScaleProgressRange(
            style()->scaleStartProgress(),
            style()->scaleEndProgress());
    assistantNotificationParameters->setOpacityProgressRange(
            style()->opacityStartProgress(),
            style()->opacityEndProgress());
}

void NotificationArea::setOutgoingLayoutTitle(const QString &title)
{
    mOutgoingLayoutTitle = title;
    outgoingNotification->setText(title);
}

QString NotificationArea::outgoingLayoutTitle() const
{
    return mOutgoingLayoutTitle;
}

void NotificationArea::setIncomingLayoutTitle(PanGesture::PanDirection direction,
                                            const QString &title)
{
    if (direction == PanGesture::PanRight)
        mRightLayoutTitle = title;
    else
        mLeftLayoutTitle = title;
}

QString NotificationArea::setIncomingLayoutTitle(PanGesture::PanDirection direction) const
{
    if (direction == PanGesture::PanRight)
        return mRightLayoutTitle;
    else
        return mLeftLayoutTitle;
}

void NotificationArea::playShowAnimation()
{
    if (!isVisible()) {
        setVisible(true);
        setOpacity(style()->initialOpacity());
    }

    incomingNotification->setVisible(true);
    assistantNotification->setVisible(true);
    outgoingNotification->setVisible(true);
    hideAnimationGroup.stop();
    hideAnimationGroup.clear();
    // show animation start from current opacity
    showAnimation.setStartValue(opacity());
    showAnimation.setEndValue(style()->visibleOpacity());
    showAnimation.start();
}

void NotificationArea::playHideAnimation(PanGesture::PanDirection result)
{
    showAnimation.stop();

    Notification *resultNotification = incomingNotification;
    if (result == PanGesture::PanNone) {
        incomingNotification->setVisible(false);
        assistantNotification->setVisible(false);
        resultNotification = outgoingNotification;
    }
    setVisible(true);
    hideAnimationGroup.clear();

    QPropertyAnimation *bgAnimation = new QPropertyAnimation(this);
    bgAnimation->setTargetObject(this);
    bgAnimation->setPropertyName("opacity");
    bgAnimation->setStartValue(opacity());
    bgAnimation->setEndValue(0.0);
    bgAnimation->setEasingCurve(style()->hideAnimationCurve());
    bgAnimation->setDuration(style()->hideAnimationDuration());

    QPropertyAnimation *itemAnimation = new QPropertyAnimation(this);
    itemAnimation->setTargetObject(resultNotification);
    itemAnimation->setPropertyName("opacity");
    itemAnimation->setStartValue(opacity());
    itemAnimation->setEndValue(0.0);
    itemAnimation->setEasingCurve(style()->hideAnimationCurve());
    itemAnimation->setDuration(style()->hideAnimationDuration());

    hideAnimationGroup.addAnimation(bgAnimation);
    hideAnimationGroup.addAnimation(itemAnimation);

    hideAnimationGroup.start();
}

void NotificationArea::onNotificationAnimationFinished()
{
    qDebug() << __PRETTY_FUNCTION__;
    showAnimation.stop();
    hideAnimationGroup.stop();
    hideAnimationGroup.clear();
    setVisible(false);
}

void NotificationArea::cancel()
{
    onNotificationAnimationFinished();
}

void NotificationArea::setProgress(qreal progress)
{
    outgoingNotificationParameters->setProgress(progress);
    incomingNotificationParameters->setProgress(progress);
    assistantNotificationParameters->setProgress(progress);
}

void NotificationArea::applyStyle()
{
    showAnimation.setEasingCurve(style()->showAnimationCurve());
    showAnimation.setDuration(style()->showAnimationDuration());
    showAnimation.setEndValue(style()->visibleOpacity());
}

void NotificationArea::reset()
{
    setOutgoingLayoutTitle("");
    setIncomingLayoutTitle(PanGesture::PanLeft, "");
    setIncomingLayoutTitle(PanGesture::PanRight, "");
}
