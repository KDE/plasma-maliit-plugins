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

#include "layoutpanner.h"
#include "mimsnapshotpixmapitem.h"
#include "mplainwindow.h"
#include "reactionmappainter.h"
#include "reactionmapwrapper.h"
#include "outgoinglayoutpanparameters.h"
#include "incominglayoutpanparameters.h"
#include "foregroundmaskpanparameters.h"
#include "notification.h"
#include "notificationarea.h"

#include <MSceneManager>
#include <MScalableImage>
#include <QPainter>
#include <QGraphicsWidget>
#include <QGraphicsSceneMouseEvent>
#include <QEasingCurve>
#include <QPauseAnimation>
#include <QSequentialAnimationGroup>
#include <QDebug>

#include <MGConfItem>
#include <limits>
#include <float.h>

namespace {
    const int MaskItemZValue = 10;
    const int NotificationAreaBackgroundZValue = 11;
}

LayoutPanner *LayoutPanner::sharedInstance = 0;

LayoutPanner::LayoutPanner(QGraphicsWidget *parent) :
    MStylableWidget(parent),
    ReactionMapPaintable(),
    mPanEnabled(true),
    panningAnimation(this, "panningPosition"),
    catchingUpAnimationGroup(this),
    result(PanGesture::PanNone),
    mPluginSwitching(false),
    outgoingLayoutItem(new MImSnapshotPixmapItem(this)),
    incomingLayoutItem(0),
    leftLayoutItem(new MImSnapshotPixmapItem(this)),
    rightLayoutItem(new MImSnapshotPixmapItem(this)),
    maskPixmapItem(new MImSnapshotPixmapItem(this)),
    sharedPixmapItem(new MImSnapshotPixmapItem(this)),
    notificationArea(new NotificationArea(this)),
    outgoingLayoutParameters(new OutgoingLayoutPanParameters(this)),
    incomingLayoutParameters(new IncomingLayoutPanParameters(this)),
    foregroundMaskPanParameters(new ForegroundMaskPanParameters(this)),
    mLayoutsProgress(0.0),
    mNotificationsProgress(0.0)
{
    connect(&panningAnimation, SIGNAL(finished()),
            this, SLOT(onPanningAnimationFinished()));

    connect(&catchingUpAnimationGroup, SIGNAL(finished()),
            this, SLOT(onCatchingUpAnimationFinished()));

    maskPixmapItem->setZValue(MaskItemZValue);

    foregroundMaskPanParameters->setOpacityFactor(style()->initialDimmingOpacity());

    setZValue(FLT_MAX);
    hide();

    outgoingLayoutItem->connectPanParameters(outgoingLayoutParameters);
    leftLayoutItem->connectPanParameters(incomingLayoutParameters);
    rightLayoutItem->connectPanParameters(incomingLayoutParameters);
    maskPixmapItem->connectPanParameters(foregroundMaskPanParameters);
}

LayoutPanner::~LayoutPanner()
{
    delete outgoingLayoutItem;
    outgoingLayoutItem = 0;
    delete leftLayoutItem;
    leftLayoutItem = 0;
    delete rightLayoutItem;
    rightLayoutItem = 0;
    incomingLayoutItem = 0;
    delete maskPixmapItem;
    maskPixmapItem = 0;
    delete sharedPixmapItem;
    sharedPixmapItem = 0;
    reset();
}

void LayoutPanner::createInstance(QGraphicsWidget *parent)
{
    Q_ASSERT(!sharedInstance);
    if (!sharedInstance) {
        sharedInstance = new LayoutPanner(parent);
    }
}

void LayoutPanner::destroyInstance()
{
    Q_ASSERT(sharedInstance);
    delete sharedInstance;
    sharedInstance = 0;
}

void LayoutPanner::setPanEnabled(bool enable)
{
    mPanEnabled = enable;
    if (!mPanEnabled && isVisible())
        cancel();
}

bool LayoutPanner::isPanEnabled() const
{
    return mPanEnabled;
}

void LayoutPanner::tryPan(PanGesture::PanDirection direction,
                          const QPoint &startPosition)
{
    if (!mPanEnabled || direction == PanGesture::PanNone)
        return;

    emit preparingLayoutPan(direction, startPos);

    startPos = mapFromScene(startPosition).toPoint();
    currentPos = startPos;
    lastMousePos = startPos;
    this->direction = direction;
    this->result = direction;
    prepare();

    show();
    notificationArea->playShowAnimation();
    // force request repaint reactionmap
    // TODO: could be removed after reactionmap have proper fix (draw according
    // Z value and regarding obscured attribute.
    signalForwarder.emitRequestRepaint();
    grabMouse();
}

void LayoutPanner::addOutgoingWidget(QGraphicsWidget *widget)
{
    if (widget)
        outgoingWidgets.append(widget);
}

void LayoutPanner::clearOutgoingWidgets()
{
    outgoingWidgets.clear();
}

void LayoutPanner::addIncomingWidget(PanGesture::PanDirection direction,
                                           QGraphicsWidget *widget)
{
    if (widget) {
        if (direction == PanGesture::PanLeft)
            leftIncomingWidgets.append(widget);

        if (direction == PanGesture::PanRight)
            rightIncomingWidgets.append(widget);
    }
}

void LayoutPanner::clearIncomingWidgets(PanGesture::PanDirection direction)
{
    if (direction == PanGesture::PanLeft)
        leftIncomingWidgets.clear();

    if (direction == PanGesture::PanRight)
        rightIncomingWidgets.clear();
}

void LayoutPanner::addSharedPixmap(QPixmap *pixmap, const QPoint &position)
{
    if (pixmap)
        sharedPixmapMap.insert(pixmap, position);
}

void LayoutPanner::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *)
{
    QRectF rect= boundingRect();
    if (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait) {
        rect = QRectF(0, 0, rect.height(), rect.width());
    }

    const MScalableImage *background = style()->backgroundImage();
    if (background) {
        background->draw(rect.toRect(), painter);
        return;
    }

    const QColor backgroundColor = style()->backgroundColor();
    const qreal opacity = qBound(qreal(0.0f), style()->backgroundOpacity(), qreal(1.0f));
    if (backgroundColor.isValid() && opacity > 0) {
        const qreal oldOpacity = painter->opacity();
        painter->setOpacity(opacity);
        painter->fillRect(rect, backgroundColor);
        painter->setOpacity(oldOpacity);
    }
}

const QPoint &LayoutPanner::panningPosition() const
{
    return currentPos;
}

/*!
 * Plays catching up animation to move keyboard layous and notifications
 * from position \a start to \a end.
 */
void LayoutPanner::goToPanningPosition(const QPoint &start, const QPoint &end)
{
    catchingUpAnimationGroup.stop();
    catchingUpAnimationGroup.clear();
    QPropertyAnimation *layoutsCatchingUpAnimation
        = new QPropertyAnimation(this, "layoutsProgress");
    QPropertyAnimation *notificationsCatchingUpAnimation
        = new QPropertyAnimation(this, "notificationsProgress");

    qreal layoutsDuration = qBound<qreal>(0.0,
                            qAbs(end.x() - start.x())
                            * style()->layoutsCatchingUpAnimationDurationMultiplier(),
                            style()->catchingUpAnimationMaximumDuration());

    qreal notificationsDuration = qBound<qreal>(0.0,
                                  qAbs(end.x() - start.x())
                                  * style()->notificationsCatchingUpAnimationDurationMultiplier(),
                                  style()->catchingUpAnimationMaximumDuration());

    bool isPortrait =
        (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait);
    qreal startProgress = isPortrait
                     ? qreal(qAbs(start.x() - startPos.x()) / size().height())
                     : qreal(qAbs(start.x() - startPos.x()) / size().width());

    qreal endProgress = isPortrait
                     ? qreal(qAbs(end.x() - startPos.x()) / size().height())
                     : qreal(qAbs(end.x() - startPos.x()) / size().width());

    layoutsCatchingUpAnimation->setDuration(layoutsDuration);
    layoutsCatchingUpAnimation->setEasingCurve(style()->layoutsCatchingUpAnimationCurve());

    QSequentialAnimationGroup *notificationsAnimationGroup
        = new QSequentialAnimationGroup(this);
    notificationsAnimationGroup->addAnimation(
        new QPauseAnimation(notificationsDuration
                            * style()->notificationsCatchingUpAnimationPause()));

    notificationsCatchingUpAnimation->setEasingCurve(
        style()->notificationsCatchingUpAnimationCurve());
    notificationsCatchingUpAnimation->setDuration(
        notificationsDuration
        * (1 - style()->notificationsCatchingUpAnimationPause()));
    notificationsAnimationGroup->addAnimation(notificationsCatchingUpAnimation);

    layoutsCatchingUpAnimation->setStartValue(startProgress);
    layoutsCatchingUpAnimation->setEndValue(endProgress);
    notificationsCatchingUpAnimation->setStartValue(startProgress);
    notificationsCatchingUpAnimation->setEndValue(endProgress);

    catchingUpAnimationGroup.addAnimation(layoutsCatchingUpAnimation);
    catchingUpAnimationGroup.addAnimation(notificationsAnimationGroup);

    // require linear transition during catching up animation 
    notificationArea->requireLinearTransition(startProgress, endProgress);
    catchingUpAnimationGroup.start();
}

void LayoutPanner::setPanningPosition(const QPoint &pos)
{
    currentPos = pos;

    bool isPortrait =
        (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait);
    qreal progress = isPortrait
                     ? qreal(qAbs(distance())) / size().height()
                     : qreal(qAbs(distance())) / size().width();

    setLayoutsProgress(progress);
    setNotificationsProgress(progress);
}

void LayoutPanner::onPanningAnimationFinished()
{
    qDebug() << __PRETTY_FUNCTION__ << ", result:" << result;

    if (mPluginSwitching) {
        foreach (QGraphicsWidget *widget, outgoingWidgets) {
            widget->show();
        }
    }

    // play animation to hide notification area if the sender is from panning animation
    if (&panningAnimation == static_cast<QPropertyAnimation *>(sender())) {
        notificationArea->setParentItem(parentWidget());
        notificationArea->setPos(
            pos() - QPointF(0,
                            notificationArea->preferredHeight()
                            - sharedPixmapItem->boundingRect().height()));
        notificationArea->playHideAnimation(result);
    }
    hide();
    setEnabled(true);
    emit layoutPanFinished(result);
    reset();
}

void LayoutPanner::onCatchingUpAnimationFinished()
{
    notificationArea->clearLinearTransition();
    // Re-enable the mutaion maybe cause notifications to change scale and opacity.
    // So call setNotificationsProgress() to update.
    setNotificationsProgress(mNotificationsProgress);
}

void LayoutPanner::applyStyle()
{
    panningAnimation.setEasingCurve(style()->panningAnimationCurve());
}

void LayoutPanner::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPoint currentMousePos = event->pos().toPoint();
    bool isPortrait =
        (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait);

    int moveDistance = 0;
    qreal endProgress = 0.0;
    if (catchingUpAnimationGroup.state() == QAbstractAnimation::Running) {
        QPropertyAnimation *animation
            = qobject_cast<QPropertyAnimation *>(catchingUpAnimationGroup.animationAt(0));
        if (animation) {
            endProgress
                = isPortrait
                  ? qAbs(currentMousePos.x() - startPos.x()) / size().height()
                  : qAbs(currentMousePos.x() - startPos.x()) / size().width();

            moveDistance
                = isPortrait
                  ? qAbs(endProgress - animation->endValue().toReal())
                    * size().height()
                  : qAbs(endProgress - animation->endValue().toReal())
                    * size().width();
        }
    } else {
        moveDistance = qAbs(currentMousePos.x() - lastMousePos.x());
    }

    if (qAbs(moveDistance) > style()->suddenMovementThreshold()
        && lastMousePos == startPos) {
        // only need catching up animation for first panning.
        goToPanningPosition(lastMousePos, currentMousePos);

    } else if (catchingUpAnimationGroup.state() == QAbstractAnimation::Running
               && qAbs(moveDistance) > style()->minimumMovementThreshold()) {
        // if catchingUpAnimation is already started,
        // adjust the end progress for catching up animation
        // TODO: use member variables with QPointer type to record animations
        // to avoid for loop and qobject_cast.
        for (int i = 0; i < catchingUpAnimationGroup.animationCount(); i++) {
            QSequentialAnimationGroup *animationGroup
                = qobject_cast<QSequentialAnimationGroup *>
                  (catchingUpAnimationGroup.animationAt(i));
            if (animationGroup) {
                for (int j = 0; j < animationGroup->animationCount(); j++) {
                    QPropertyAnimation *animation
                        = qobject_cast<QPropertyAnimation *>
                          (animationGroup->animationAt(i));
                    if (animation) {
                        // require new linear transition end progress
                        notificationArea->requireLinearTransition(
                            animation->startValue().toReal(), endProgress);
                        animation->setEndValue(endProgress);
                    }
                }
            } else {
                QPropertyAnimation *animation
                = qobject_cast<QPropertyAnimation *>
                  (catchingUpAnimationGroup.animationAt(i));
                if (animation)
                    animation->setEndValue(endProgress);
            }
        }

    } else if (catchingUpAnimationGroup.state() != QAbstractAnimation::Running) {
        catchingUpAnimationGroup.stop();
        catchingUpAnimationGroup.clear();
        setPanningPosition(currentMousePos);
    }
    // else ignore the small movements.

    lastMousePos = currentMousePos;
}

void LayoutPanner::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // eat mouse press event
    event->accept();
}

void LayoutPanner::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setPanningPosition(event->pos().toPoint());
    finalize();
    event->accept();
}

void LayoutPanner::prepare()
{
    bool isPortrait =
        (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait);
    QRectF rect;
    foreach (const QGraphicsWidget *widget, outgoingWidgets) {
        rect |= widget->sceneBoundingRect();
    }

    qreal posY = 0;
    if (isPortrait) {
        posY = MPlainWindow::instance()->visibleSceneSize(M::Landscape).width()
                - rect.width();
    } else {
        posY = MPlainWindow::instance()->visibleSceneSize(M::Landscape).height()
                - rect.height();
    }
    QPoint pos(0, posY);

    switch(direction) {
    case PanGesture::PanLeft:
        incomingLayoutItem = rightLayoutItem;
        break;
    case PanGesture::PanRight:
        incomingLayoutItem = leftLayoutItem;
        break;
    default:
        break;
    }

    notificationArea->prepareNotifications(direction);
    outgoingLayoutItem->grabScreen(rect.toRect());
    sharedPixmapItem->grabPixmaps(sharedPixmapMap);

    QMap<QPixmap*, QPoint>::const_iterator it = sharedPixmapMap.constBegin();
    while (it != sharedPixmapMap.constEnd() ) {
        if (it.key()) {
            if (isPortrait) {
                rect |= QRect(QPoint(it.value().y(), 0), it.key()->size());
            } else {
                rect |= QRect(it.value(), it.key()->size());
            }
            pos.setY(qMin(it.value().y(), pos.y()));
        }
        it++;
    }

    resize(rect.size());
    setPos(pos);

    preparePanningItems();
    notificationArea->setParentItem(this);
    notificationArea->setZValue(NotificationAreaBackgroundZValue);
    notificationArea->setPos(QPointF(0,
                                     sharedPixmapItem->boundingRect().height()
                                     - notificationArea->preferredHeight()));

    if (mPluginSwitching) {
        foreach (QGraphicsWidget *widget, outgoingWidgets) {
            widget->hide();
        }
    }
}

void LayoutPanner::prepareOrientationChange()
{
    cancel();
}

void LayoutPanner::finalizeOrientationChange()
{
    reset();
}

void LayoutPanner::grabIncomingSnapshot()
{
    leftLayoutItem->grabWidgets(leftIncomingWidgets);
    leftLayoutItem->setVisible(false);

    rightLayoutItem->grabWidgets(rightIncomingWidgets);
    rightLayoutItem->setVisible(false);
}

void LayoutPanner::preparePanningItems()
{
    const int keyboardWidth
        = (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait)
          ? rect().size().height()
          : rect().size().width();

    QPointF outgoingLayoutItemFromPos
        = QPointF(0, sharedPixmapItem->boundingRect().height());
    outgoingLayoutItem->updateOpacity(style()->outgoingLayoutFromOpacity());
    outgoingLayoutItem->updatePos(outgoingLayoutItemFromPos);
    outgoingLayoutItem->setVisible(true);
    QPointF outgoingPanningItemToPos =
        (direction == PanGesture::PanRight)
        ? QPointF(keyboardWidth, outgoingLayoutItemFromPos.y())
        : QPointF(-keyboardWidth, outgoingLayoutItemFromPos.y());

    outgoingLayoutParameters->setPositionRange(outgoingLayoutItemFromPos,
                                                outgoingPanningItemToPos);

    outgoingLayoutParameters->setOpacityRange(style()->outgoingLayoutFromOpacity(),
                                              style()->outgoingLayoutToOpacity());

    outgoingLayoutParameters->setOpacityProgressRange(
        style()->outgoingLayoutOpacityStartProgress(),
        style()->outgoingLayoutOpacityEndProgress());

    QPointF incomingLayoutItemFromPos;
    int incomingPixmapPosX = (direction == PanGesture::PanRight)
                         ? (-keyboardWidth)
                         : (keyboardWidth );

    int incomingPixmapPosY = outgoingLayoutItem->boundingRect().height()
                         - incomingLayoutItem->boundingRect().height();
    incomingPixmapPosY = (incomingPixmapPosY < 0) ? 0 : incomingPixmapPosY;
    incomingPixmapPosY += sharedPixmapItem->boundingRect().height();

    incomingLayoutItemFromPos = QPointF(incomingPixmapPosX, incomingPixmapPosY);
    incomingLayoutItem->updateOpacity(style()->incomingLayoutFromOpacity());
    incomingLayoutItem->updatePos(incomingLayoutItemFromPos);
    incomingLayoutItem->setVisible(true);

    QPointF incomingLayoutItemToPos = QPointF(0, incomingLayoutItemFromPos.y());

    incomingLayoutParameters->setPositionRange(incomingLayoutItemFromPos,
                                               incomingLayoutItemToPos);

    incomingLayoutParameters->setOpacityRange(style()->incomingLayoutFromOpacity(),
                                              style()->incomingLayoutToOpacity());

    sharedPixmapItem->setPos(QPointF(0, 0));

    updateDimmingPixmapItem();
    maskPixmapItem->setPos(outgoingLayoutItemFromPos);
}

void LayoutPanner::updateDimmingPixmapItem()
{
    const QRectF rect = outgoingLayoutItem->boundingRect();
    QPixmap dimmingPixmap(outgoingLayoutItem->pixmap().size());
    dimmingPixmap.fill(Qt::transparent);
    QPainter dimmingPainter(&dimmingPixmap);

    QRadialGradient radialGrad(QPointF(rect.width()/2,
                                       rect.height()/2),
                               rect.width()/2);
    radialGrad.setColorAt(0, Qt::transparent);
    radialGrad.setColorAt(1, Qt::black);

    dimmingPainter.fillRect(0, 0, dimmingPixmap.width(), dimmingPixmap.height(),
                            QBrush(radialGrad));

    if (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait) {
        QTransform portraitTransform;
        portraitTransform.rotate(90);
        dimmingPixmap = dimmingPixmap.transformed(portraitTransform);
    }

    maskPixmapItem->setPixmap(dimmingPixmap);
}

int LayoutPanner::distance() const
{
    return currentPos.x() - startPos.x();
}

bool LayoutPanner::isSwitchingPlugin() const
{
    return mPluginSwitching;
}

void LayoutPanner::setSwitchingPlugin(bool flag)
{
    mPluginSwitching = flag;
}

void LayoutPanner::reset()
{
    catchingUpAnimationGroup.stop();
    catchingUpAnimationGroup.clear();

    mPluginSwitching = false;
    result = PanGesture::PanNone;
    outgoingWidgets.clear();
    if (incomingLayoutItem)
        incomingLayoutItem->setVisible(false);
    incomingLayoutItem = 0;
    sharedPixmapMap.clear();
    outgoingLayoutParameters->reset();
    incomingLayoutParameters->reset();
    foregroundMaskPanParameters->reset();
    mLayoutsProgress= 0.0;
    mNotificationsProgress = 0.0;
}

bool LayoutPanner::isPanningLayouts() const
{
    return isVisible();
}

void LayoutPanner::cancel()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (isVisible()) {
        setEnabled(false);
        result = PanGesture::PanNone;
        onPanningAnimationFinished();
    }
    notificationArea->cancel();
}


void LayoutPanner::finalize()
{
    if (!isVisible())
        return;

    const bool isPortrait =
        (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait);
    QPointF startValue = currentPos;

    if (catchingUpAnimationGroup.state() == QAbstractAnimation::Running) {
        qreal currentProgress = 0.0;
        for (int i = 0; i < catchingUpAnimationGroup.animationCount(); i++) {
            QPropertyAnimation *animation
                = qobject_cast<QPropertyAnimation *>(catchingUpAnimationGroup.animationAt(i));
            if (animation) {
                currentProgress = animation->currentValue().toReal();
                break;
            }
        }
        const int keyboardWidth = isPortrait ? size().height() : size().width();
        startValue.setX((direction == PanGesture::PanRight)
                        ? currentProgress * keyboardWidth
                        : (1- currentProgress) * keyboardWidth);

        catchingUpAnimationGroup.stop();
        catchingUpAnimationGroup.clear();
    }
    panningAnimation.setStartValue(startValue);

    QPoint endPos(0, 0);
    if (qAbs(distance()) > style()->commitThreshold()) {
        // change language
        if (isPortrait)
            if (direction == PanGesture::PanRight) {
                endPos.setX(size().height());
            } else {
                endPos.setX(0);
            }
        else {
            if (direction == PanGesture::PanRight) {
                endPos.setX(size().width());
            } else {
                endPos.setX(0);
            }
        }
        result = direction;
        panningAnimation.setDuration(style()->panningAnimationDuration());
    } else {
        // return to outgoing language
        endPos = startPos;
        result = PanGesture::PanNone;
        panningAnimation.setDuration(style()->panningAnimationDuration() / 2);
    }

    setEnabled(false);

    panningAnimation.setEndValue(endPos);
    panningAnimation.start();
}

void LayoutPanner::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
#ifndef HAVE_REACTIONMAP
    Q_UNUSED(reactionMap);
    Q_UNUSED(view);
    return;
#else
    reactionMap->setTransform(this, view);
    reactionMap->setInactiveDrawingValue();

    QPointF position(pos());
    if (MPlainWindow::instance()->sceneManager()->orientation() == M::Portrait) {
        position = QPointF(position.y(), position.x());
    }

    reactionMap->fillRectangle(position.x(), position.y(),
                               boundingRect().width(),
                               boundingRect().height());

#endif // HAVE_REACTIONMAP
}

bool LayoutPanner::isPaintable() const
{
    return isVisible();
}

void LayoutPanner::setOutgoingLayoutTitle(const QString &title)
{
    notificationArea->setOutgoingLayoutTitle(title);
}

void LayoutPanner::setIncomingLayoutTitle(PanGesture::PanDirection direction,
                                            const QString &title)
{
    notificationArea->setIncomingLayoutTitle(direction,
                                                       title);
}

qreal LayoutPanner::layoutsProgress() const
{
    return mLayoutsProgress;
}

void LayoutPanner::setLayoutsProgress(qreal progress)
{
    mLayoutsProgress = progress;
    outgoingLayoutParameters->setProgress(progress);
    incomingLayoutParameters->setProgress(progress);
    foregroundMaskPanParameters->setProgress(progress);
}

qreal LayoutPanner::notificationsProgress() const
{
    return mNotificationsProgress;
}

void LayoutPanner::setNotificationsProgress(qreal progress)
{
    mNotificationsProgress = progress;
    notificationArea->setProgress(progress);
}
