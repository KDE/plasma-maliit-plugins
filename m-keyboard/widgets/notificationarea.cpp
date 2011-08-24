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
      outgoingNotificationParameters(new OutgoingNotificationPanParameters(this)),
      incomingNotificationParameters(new IncomingNotificationPanParameters(this)),
      assistantNotificationParameters(new AssistantNotificationPanParameters(this)),
      hideAnimationGroup(this),
      showAnimation(this, "opacity"),
      outgoingNotificationFromScale(0.0),
      outgoingNotificationToScale(0.0),
      incomingNotificationFromScale(0.0),
      incomingNotificationToScale(0.0),
      assistantNotificationFromScale(0.0),
      assistantNotificationToScale(0.0),
      linearTransitionStartProgress(0.0),
      linearTransitionEndProgress(0.0)
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
    this->direction = direction;

    // prepare the panning parameters for notifications
    const int screenWidth
        = (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait)
          ? MPlainWindow::instance()->visibleSceneSize(M::Landscape).height()
          : MPlainWindow::instance()->visibleSceneSize(M::Landscape).width();

    qreal notificationMaximumWidth
        = style()->notificationMaximumWidth();

    outgoingNotification->setMaximumTextWidth(notificationMaximumWidth);
    incomingNotification->setMaximumTextWidth(notificationMaximumWidth);
    assistantNotification->setMaximumTextWidth(notificationMaximumWidth);

    outgoingNotificationFromScale
        = qBound<qreal>(0, qreal(notificationMaximumWidth / outgoingNotification->preferredWidth()), 1.0);

    outgoingNotificationToScale = style()->smallSizeScaleFactor();

    outgoingNotificationFromPos
        = QPointF((screenWidth - outgoingNotification->preferredWidth()
                   * outgoingNotificationFromScale) / 2,
                  preferredHeight()
                  - outgoingNotification->preferredHeight()
                  * (outgoingNotificationFromScale / 2.0f + 0.5f));

    qreal outgoingNotificationToPosX
        = (direction == PanGesture::PanRight)
          ? screenWidth
          : - outgoingNotification->preferredWidth()
            * outgoingNotificationToScale;

    qreal outgoingNotificationToPosY
        = preferredHeight()
          - outgoingNotification->preferredHeight()
          * (outgoingNotificationToScale / 2.0f + 0.5f);

    outgoingNotificationToPos
        = QPointF(outgoingNotificationToPosX, outgoingNotificationToPosY);

    outgoingNotification->updateScale(outgoingNotificationFromScale);
    outgoingNotification->updateOpacity(1.0);
    outgoingNotification->updatePos(outgoingNotificationFromPos);
    outgoingNotification->setVisible(false);

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

    incomingNotificationFromScale = outgoingNotificationToScale;

    incomingNotificationToScale
        = qBound<qreal>(0, qreal(notificationMaximumWidth / incomingNotification->preferredWidth()), 1.0);

    qreal fromPosX = 0;
    if (incomingNotification->preferredWidth() * incomingNotificationFromScale
        > leftAndRightSpace) {

       fromPosX
        = (direction == PanGesture::PanRight)
          ? leftAndRightSpace
            - incomingNotification->preferredWidth() * incomingNotificationFromScale
          : screenWidth - leftAndRightSpace;
    } else {

       fromPosX
        = (direction == PanGesture::PanRight)
          ? 0
          : screenWidth - incomingNotification->preferredWidth() * incomingNotificationFromScale;
    }

    qreal fromPosY = preferredHeight()
                     - incomingNotification->preferredHeight()
                     * (incomingNotificationFromScale / 2.0f + 0.5f);

    incomingNotificationFromPos = QPointF(fromPosX, fromPosY);

    qreal toPosX = (screenWidth - incomingNotification->preferredWidth()
                    * incomingNotificationToScale) / 2;

    qreal toPosY = preferredHeight()
                   - incomingNotification->preferredHeight()
                   * (incomingNotificationToScale / 2.0f + 0.5f);

    incomingNotificationToPos = QPointF(toPosX, toPosY);

    incomingNotification->updateScale(incomingNotificationFromScale);
    incomingNotification->updatePos(incomingNotificationFromPos);
    incomingNotification->updateOpacity(style()->initialEdgeOpacity());
    incomingNotification->setVisible(false);

    assistantNotificationFromScale = outgoingNotificationToScale;
    assistantNotificationToScale = 
        assistantNotificationFromScale * outgoingNotificationToScale;

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
          - assistantNotification->preferredHeight()
          * (assistantNotificationFromScale / 2.0f + 0.5f);
    qreal assistantNotificationToPosY
        = preferredHeight()
          - assistantNotification->preferredHeight()
          * (outgoingNotificationToScale / 2.0f + 0.5f);

    assistantNotificationFromPos
        = QPointF(assistantNotificationFromPosX, assistantNotificationFromPosY);

    qreal outgoingNotificationXMoveMent
        = outgoingNotificationToPos.x() - outgoingNotificationFromPos.x();
    assistantNotificationToPos
        = QPointF(assistantNotificationFromPos.x() + outgoingNotificationXMoveMent,
                  assistantNotificationToPosY);

    assistantNotification->updateScale(assistantNotificationFromScale);
    assistantNotification->updateOpacity(style()->initialEdgeOpacity());
    assistantNotification->updatePos(assistantNotificationFromPos);
    assistantNotification->setVisible(false);

    if (direction == PanGesture::PanLeft) {
        outgoingNotificationParameters->setAvoidOverlappingPanParameters(
            incomingNotificationParameters, direction,
            outgoingNotification->preferredWidth());

        assistantNotificationParameters->setKeepDistancePanParameters(
            outgoingNotificationParameters, direction,
            assistantNotification->preferredWidth(),
            (outgoingNotification->pos().x()
            - assistantNotification->pos().x()
            - assistantNotification->preferredWidth()
            * assistantNotificationParameters->scale()));

    } else {
        outgoingNotificationParameters->setAvoidOverlappingPanParameters(
            incomingNotificationParameters, direction,
            incomingNotification->preferredWidth());

        assistantNotificationParameters->setKeepDistancePanParameters(
            outgoingNotificationParameters, direction,
            outgoingNotification->preferredWidth(),
            (assistantNotification->pos().x()
            - outgoingNotification->pos().x()
            - outgoingNotification->preferredWidth()
            * outgoingNotificationParameters->scale()));
    }

    initPanParameters();
    outgoingNotificationParameters->reset();
    incomingNotificationParameters->reset();
    assistantNotificationParameters->reset();
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
    incomingNotificationParameters->setProgress(progress);
    outgoingNotificationParameters->setProgress(progress);
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

void NotificationArea::requireLinearTransition(qreal startProgress,
                                               qreal endProgress)
{
    if (linearTransitionStartProgress == startProgress
        && linearTransitionEndProgress == endProgress)
        return;

    linearTransitionStartProgress = startProgress;
    linearTransitionEndProgress = endProgress;

    initPanParameters();
    if (endProgress > style()->outgoingScaleMutationProgress()
        || endProgress > style()->outgoingOpacityMutationProgress()) {
        // adjust scale and opacity changing range and value
        // to make the animation transition smooth.
        incomingNotificationParameters->setPositionRange(
            incomingNotificationParameters->positionAt(startProgress),
            incomingNotificationParameters->positionAt(endProgress));
        incomingNotificationParameters->setPositionProgressRange(
            startProgress, endProgress);

        incomingNotificationParameters->setScaleRange(
            incomingNotificationParameters->scaleAt(startProgress),
            incomingNotificationParameters->scaleAt(endProgress));
        incomingNotificationParameters->setScaleProgressRange(
            startProgress, endProgress);

        incomingNotificationParameters->setOpacityRange(
        incomingNotificationParameters->opacityAt(startProgress),
        incomingNotificationParameters->opacityAt(endProgress));
        incomingNotificationParameters->setOpacityProgressRange(
            startProgress, endProgress);

        outgoingNotificationParameters->setPositionRange(
        outgoingNotificationParameters->positionAt(startProgress),
        outgoingNotificationParameters->positionAt(endProgress));
        outgoingNotificationParameters->setPositionProgressRange(
            startProgress, endProgress);

        outgoingNotificationParameters->setScaleRange(
            outgoingNotificationParameters->scaleAt(startProgress),
            outgoingNotificationParameters->scaleAt(endProgress));
        outgoingNotificationParameters->setScaleProgressRange(
            startProgress, endProgress);

        outgoingNotificationParameters->setOpacityRange(
        outgoingNotificationParameters->opacityAt(startProgress),
            outgoingNotificationParameters->opacityAt(endProgress));
        outgoingNotificationParameters->setOpacityProgressRange(
            startProgress, endProgress);
    }

    // disable scale and opacity mutation.
    if (linearTransitionStartProgress != linearTransitionEndProgress) {
        outgoingNotificationParameters->setScaleMutation(0.00, 0.00);
        outgoingNotificationParameters->setOpacityMutation(0.00, 0.00);
        incomingNotificationParameters->setScaleMutation(0.00, 0.00);
        incomingNotificationParameters->setOpacityMutation(0.00, 0.00);
    }
}

void NotificationArea::clearLinearTransition()
{
    requireLinearTransition(0.0, 0.0);
}

/*!
 * Initialize the PanParameters for notifications.
 */
void NotificationArea::initPanParameters()
{
    outgoingNotificationParameters->setScaleRange(outgoingNotificationFromScale,
                                                  outgoingNotificationToScale);
    outgoingNotificationParameters->setOpacityRange(1.0, 0.0);
    outgoingNotificationParameters->setPositionRange(
        outgoingNotificationFromPos, outgoingNotificationToPos);

    incomingNotificationParameters->setScaleRange(incomingNotificationFromScale,
                                                  incomingNotificationToScale);
    incomingNotificationParameters->setOpacityRange(style()->initialEdgeOpacity(),
                                                    1.0);
    incomingNotificationParameters->setPositionRange(
        incomingNotificationFromPos, incomingNotificationToPos);

    assistantNotificationParameters->setPositionRange(
        assistantNotificationFromPos, assistantNotificationToPos);
    assistantNotificationParameters->setScaleRange(assistantNotificationFromScale,
                                                   assistantNotificationToScale);
    assistantNotificationParameters->setOpacityRange(style()->initialEdgeOpacity(),
                                                     0.0);

    // set scale and opacity mutation
    outgoingNotificationParameters->setScaleMutation(
        style()->outgoingScaleMutationProgress(),
        style()->outgoingScaleMutation());

    outgoingNotificationParameters->setOpacityMutation(
        style()->outgoingOpacityMutationProgress(),
        style()->outgoingOpacityMutation());

    qreal incomingScaleMutation =
        qMin<qreal>(style()->incomingScaleMutation(),
                    qBound<qreal>(0,
                                  qreal(style()->notificationMaximumWidth()
                                  / incomingNotification->preferredWidth()),
                                  1.0));
    incomingNotificationParameters->setScaleMutation(
        style()->incomingScaleMutationProgress(),
        incomingScaleMutation);
    incomingNotificationParameters->setOpacityMutation(
        style()->incomingOpacityMutationProgress(),
        style()->incomingOpacityMutation());

    outgoingNotificationParameters->setPositionProgressRange(
            style()->outgoingPositionStartProgress(),
            style()->outgoingPositionEndProgress());
    outgoingNotificationParameters->setScaleProgressRange(
            style()->outgoingScaleStartProgress(),
            style()->outgoingScaleEndProgress());
    outgoingNotificationParameters->setOpacityProgressRange(
            style()->outgoingOpacityStartProgress(),
            style()->outgoingOpacityEndProgress());

    incomingNotificationParameters->setPositionProgressRange(
            style()->incomingPositionStartProgress(),
            style()->incomingPositionEndProgress());
    incomingNotificationParameters->setScaleProgressRange(
            style()->incomingScaleStartProgress(),
            style()->incomingScaleEndProgress());
    incomingNotificationParameters->setOpacityProgressRange(
            style()->incomingOpacityStartProgress(),
            style()->incomingOpacityEndProgress());

    assistantNotificationParameters->setPositionProgressRange(
            style()->outgoingPositionStartProgress(),
            style()->outgoingPositionEndProgress());
    assistantNotificationParameters->setScaleProgressRange(
            style()->outgoingScaleStartProgress(),
            style()->outgoingScaleEndProgress());
    assistantNotificationParameters->setOpacityProgressRange(
            style()->outgoingOpacityStartProgress(),
            style()->outgoingOpacityEndProgress());
}
