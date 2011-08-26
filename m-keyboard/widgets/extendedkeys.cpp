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

#include "extendedkeys.h"
#include "magnifierhost.h"
#include "reactionmapwrapper.h"

#include <regiontracker.h>
#include <layoutdata.h>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMargins>
#include <QAnimationGroup>
#include <MFeedback>
#include <MCancelEvent>
#include <QTouchEvent>

namespace {
    QSize queryKeyAreaSize(const MImAbstractKeyArea *keyArea,
                           int keyCount)
    {
        const MImAbstractKeyAreaStyleContainer &style(keyArea->baseStyle());

        return QSize(style->keyWidthMediumFixed() * keyCount
                     + style->keyMarginLeft() * (keyCount - 1)
                     + style->keyMarginRight() * (keyCount - 1)
                     + style->paddingLeft()
                     + style->paddingRight(),
                     keyArea->size().height());
    }

    /*
     * Align given keyArea to put interspace between some keys
     * on top of origin position
     */
    void alignExtendedKeyArea(MImAbstractKeyArea *keyArea,
                      int keyCount,
                      const QPointF &origin,
                      int extendedKeysOffset,
                      const QMargins &safetyMargins)
    {
        if (!keyArea || keyCount < 2) {
            return;
        }

        const MImAbstractKeyAreaStyleContainer &style(keyArea->baseStyle());
        const int paddingL = style->paddingLeft();
        const int keyWidth = style->keyWidthMediumFixed();
        const int keyMarginL = style->keyMarginLeft();
        const int keyMarginR = style->keyMarginRight();
        // Position of interspace between first and second key.
        // This line actually depends in MImKey implementation.
        const int firstPosCandidate = paddingL + keyWidth + keyMarginR;

        // Select interspace which should be placed on top of origin.
        // n==0 points to interspace between first and second keys
        int n = qRound((origin.x() - keyArea->pos().x()
                        - firstPosCandidate)
                       / (keyWidth + keyMarginL + keyMarginR));
        // We do not want to put right border of last key on top
        // of origin position, so upper bound is "keyCount - 2"
        // instead of "keyCount -1"
        n = qBound(0, n, keyCount - 2);

        const QPointF correction(firstPosCandidate
                                 + n * (keyWidth + keyMarginL + keyMarginR),
                                 keyArea->boundingRect().bottom());

        const QPointF pos(origin
                          - correction
                          + QPointF(0, extendedKeysOffset));

        MagnifierHost::applyConstrainedPosition(keyArea, keyArea->parentItem(), pos,
                                                safetyMargins);
    }
}

ExtendedKeys::ExtendedKeys(MagnifierHost *newHost,
                           MImAbstractKeyArea *newMainArea)
    : MImOverlay()
    , ReactionMapPaintable()
    , host(newHost)
    , mainArea(newMainArea)
    , hideOnNextMouseRelease(false)
    , showAnimation(this, "magnitude")
    , currentMagnitude(1)
    , isInitialTouchPointPrimary(false)
    , followedTouchPointId(0)
    , followedTouchPointOrigin(MainKeyboardArea)
    , releasedTouchPointId(-1)
    , releasedTouchPointOrigin(MainKeyboardArea)
{
    setObjectName("ExtendedKeys"); // needed by MATTI (but useful otherwise too)
    RegionTracker::instance().addRegion(*this);

    setFlags(QGraphicsItem::ItemHasNoContents);
    setParent(newHost);

    showAnimation.setStartValue(0.0f);
    showAnimation.setEndValue(1.0f);
    connect(&showAnimation, SIGNAL(finished()),
                            SLOT(handleShowAnimationFinished()));
}

ExtendedKeys::~ExtendedKeys()
{}

bool ExtendedKeys::isPaintable() const
{
    return isVisible();
}

bool ExtendedKeys::isFullScreen() const
{
    // It acts as fullscreen
    return true;
}

void ExtendedKeys::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
    // Cast is needed for MImAbstractKeyArea * caused by protected reimplementation
    static_cast<MImAbstractKeyArea *>(extKeysArea.get())->drawReactiveAreas(reactionMap, view);
}

void ExtendedKeys::showExtendedArea(const QPointF &origin,
                                    const QPointF &tappedScenePos,
                                    const QString &labels,
                                    int touchPointId,
                                    bool touchPointIsPrimary)
{
    LayoutData::SharedLayoutSection section(new LayoutSection(labels));

    // Custom haptic feedback for this popup being appeared
    MFeedback::play("priority2_vkb_popup_press");
    // TODO: disable swipe gestures from extended keys area
    extKeysArea.reset(ExtendedKeysArea::create(section, this));
    extKeysArea->setPanningEnabled(false);
    extKeysArea->setStyleName("ExtendedKeys");

    // Send clicked signal as if it was from mainArea.
    connect(extKeysArea.get(), SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)),
            mainArea,          SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)));

    connect(extKeysArea.get(), SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)),
            host,              SLOT(hide()));

    connect(extKeysArea.get(), SIGNAL(displayExited()),
            host,              SLOT(hide()));

    connect(extKeysArea.get(), SIGNAL(displayExited()), SLOT(handleHide()));

    extKeysArea->resize(queryKeyAreaSize(extKeysArea.get(), extKeysArea->maxColumns()));
    extKeysArea->setParentItem(this);
    extKeysArea->setFlags(QGraphicsItem::ItemStacksBehindParent);

    hideOnNextMouseRelease = false;
    ignoredTouchPoints.clear();

    // Through the main area, we can access the VKB coordinate system:
    const QPointF originMapped(mainArea->mapToItem(this, origin));
    const QPointF correction(extKeysArea->boundingRect().center().x(),
                             extKeysArea->boundingRect().bottom());
    const QPointF extKeysPos(originMapped
                             - correction
                             + QPointF(0, host->style()->extendedKeysOffset()));

    const MKeyboardMagnifierStyleContainer &style = host->style();
    QMargins safetyMargins(style->safetyMarginLeft(),  MagnifierHost::InvalidMargin,
                           style->safetyMarginRight(), MagnifierHost::InvalidMargin);
    MagnifierHost::applyConstrainedPosition(extKeysArea.get(), extKeysArea->parentItem(), extKeysPos,
                                            safetyMargins);

    if (extKeysArea->pos() != extKeysPos) {
        alignExtendedKeyArea(extKeysArea.get(), labels.count(), originMapped,
                             host->style()->extendedKeysOffset(), safetyMargins);
    }

    showAnimation.setEasingCurve(host->style()->extendedKeysShowEasingCurve());
    showAnimation.setDuration(host->style()->extendedKeysShowDuration());

    // Convert anchorPoint into coordinates of extended keys area to simplify
    // calculations for animation. We do it just here, because extKeys area
    // is placed into final position and won't be moved anymore.
    anchorPoint = extKeysArea->mapFromScene(anchorPoint);
    setMagnitude(showAnimation.startValue().value<qreal>());
    show();

    // Redirect mouse and touch events with the followed touch point ID to
    // extKeyArea until the touch point is released. If extKeyArea
    // remains visible, redirect mouse and touch events of the first
    // pressed touch point after the initial touch point and ignore
    // every new touch points until extKeyArea is hidden or touch point
    // is released. First followed mouse event or touch point always
    // comes from main keyboard area.
    followedTouchPointId = touchPointId;
    followedTouchPointOrigin = MainKeyboardArea;
    releasedTouchPointId = -1;

    isInitialTouchPointPrimary = touchPointIsPrimary;
    this->tappedScenePos = tappedScenePos;

    // Instead of grabbing mouse, filter and redirect the events here
    mainArea->installEventFilter(this);

}

MImAbstractKeyArea * ExtendedKeys::extendedKeysArea() const
{
    return extKeysArea.get();
}

void ExtendedKeys::handleHide()
{
    mainArea->removeEventFilter(this);
}

void ExtendedKeys::handleShowAnimationFinished()
{
    // Only send initial mouse press if touch is not released
    // while show animation is running.
    if (!hideOnNextMouseRelease) {
        // Generate first press or touch event and send it to extKeysArea
        // if multitouch is enabled.
        if (acceptTouchEvents()) {
            QTouchEvent::TouchPoint tp;
            tp.setId(followedTouchPointId);
            if (isInitialTouchPointPrimary) {
                tp.setState(Qt::TouchPointPressed | Qt::TouchPointPrimary);
            } else {
                tp.setState(Qt::TouchPointPressed);
            }
            tp.setPos(extKeysArea->mapFromScene(tappedScenePos));
            tp.setScenePos(tappedScenePos);
            tp.setLastPos(tp.pos());
            tp.setLastScenePos(tp.scenePos());
            QList<QTouchEvent::TouchPoint> tpList;
            tpList.append(tp);
            QTouchEvent touchEvent(QEvent::TouchBegin, QTouchEvent::TouchScreen, Qt::NoModifier, tp.state(), tpList);
            scene()->sendEvent(extKeysArea.get(), &touchEvent);
        }
    
        // Generate mouse event only if touch point is primary
        if (isInitialTouchPointPrimary) {
            QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
            press.setPos(extKeysArea->mapFromScene(tappedScenePos));
            press.setLastPos(press.pos());
            press.setScenePos(tappedScenePos);
            press.setLastScenePos(press.scenePos());
            scene()->sendEvent(extKeysArea.get(), &press);
        }
    }

    // Update the reaction maps right now
    signalForwarder.emitRequestRepaint();
    // Update the reaction maps if the popup disappears
    connect(extKeysArea.get(), SIGNAL(displayExited()),
            &signalForwarder, SIGNAL(requestRepaint()));
}

bool ExtendedKeys::handleTouchEvent(QTouchEvent *event, QGraphicsItem *originalReceiver, EventOrigin from)
{
    bool eaten = false;

    QList<QTouchEvent::TouchPoint> extKeysAreaTouchPoint;
    QList<QTouchEvent::TouchPoint> receiverTouchPoints;
    Qt::TouchPointStates receiverState;

    foreach (const QTouchEvent::TouchPoint &tp, event->touchPoints()) {
        switch (tp.state() & Qt::TouchPointStateMask) {
        case Qt::TouchPointPressed:
            if (followedTouchPointId < 0) {
                // Start redirecting the first touch point after initial
                // touch point has been released.
                followedTouchPointId = tp.id();
                followedTouchPointOrigin = from;
                releasedTouchPointId = -1;
            } else if (tp.id() != followedTouchPointId
                       || followedTouchPointOrigin != from) {
                // Ignore any new touch points until extended keys area
                // is hidden.
                // Note that new touch points pressed on top of mainArea should be
                // eaten by MImOverlay but because of bug NB#248227 they actually
                // come from mainArea.
                ignoredTouchPoints[from].append(tp.id());
            }

            if (tp.id() == followedTouchPointId
                && followedTouchPointOrigin == from) {
                extKeysAreaTouchPoint.append(QTouchEvent::TouchPoint(tp));
            } else if (!ignoredTouchPoints[from].contains(tp.id())) {
                receiverTouchPoints.append(QTouchEvent::TouchPoint(tp));
                receiverState |= tp.state();
            }
            break;

        case Qt::TouchPointMoved:
        case Qt::TouchPointStationary:
            if (tp.id() == followedTouchPointId
                && followedTouchPointOrigin == from) {
                extKeysAreaTouchPoint.append(QTouchEvent::TouchPoint(tp));
            } else if (!ignoredTouchPoints[from].contains(tp.id())) {
                receiverTouchPoints.append(QTouchEvent::TouchPoint(tp));
                receiverState |= tp.state();
            }
            break;

        case Qt::TouchPointReleased:
            if (tp.id() == followedTouchPointId
                && followedTouchPointOrigin == from) {
                extKeysAreaTouchPoint.append(QTouchEvent::TouchPoint(tp));

                // If multitouch is enabled, mouse event that extKeysArea might be
                // interested in can come after touch event. We want to redirect
                // that mouse event to extKeysArea.
                releasedTouchPointId = followedTouchPointId;
                releasedTouchPointOrigin = followedTouchPointOrigin;
                followedTouchPointId = -1;

                if (!hideOnNextMouseRelease) {
                    // Initial touch point is released
                    hideOnNextMouseRelease = true;
                } else {
                    // Second followed touch point is released, hide
                    // extended keys area
                    QMetaObject::invokeMethod(host, "hide", Qt::QueuedConnection);
                }
            } else if (tp.id() == releasedTouchPointId
                       && releasedTouchPointOrigin == from) {
                // Release logic already handled at mouse event handler.
                // Just redirect the touch event to extKeyArea.
                releasedTouchPointId = -1;
                extKeysAreaTouchPoint.append(QTouchEvent::TouchPoint(tp));
            } else if (!ignoredTouchPoints[from].contains(tp.id())) {
                receiverTouchPoints.append(QTouchEvent::TouchPoint(tp));
                receiverState |= tp.state();
            }

            if (ignoredTouchPoints[from].contains(tp.id())) {
                ignoredTouchPoints[from].removeOne(tp.id());
            }
            break;

        default:
            break;
        }
    }

    if (!extKeysAreaTouchPoint.isEmpty()) {
        if (showAnimation.state() != QAbstractAnimation::Stopped) {
            // Do not forward events to extKeysArea until animation has
            // finished. Instead, only update initial press position.
            tappedScenePos = extKeysAreaTouchPoint[0].scenePos();
        } else {
            QEvent::Type type = event->type();
    
            // Make sure that TouchBegin and TouchEnd are reported correctly
            if (extKeysAreaTouchPoint[0].state() & Qt::TouchPointPressed) {
                type = QEvent::TouchBegin;
            } else if (extKeysAreaTouchPoint[0].state() & Qt::TouchPointReleased) {
                type = QEvent::TouchEnd;
            }
    
            QTouchEvent tEvent(type, QTouchEvent::TouchScreen, Qt::NoModifier, extKeysAreaTouchPoint[0].state(), extKeysAreaTouchPoint);
            scene()->sendEvent(extKeysArea.get(), &tEvent);
        }
        eaten = true;
    }

    // Only resend touch event if the original touch event was modified
    // to avoid infinite recursion.
    if (originalReceiver && !receiverTouchPoints.isEmpty()
        && receiverTouchPoints.size() != event->touchPoints().size()) {
        QTouchEvent tEvent(event->type(), QTouchEvent::TouchScreen, Qt::NoModifier, receiverState, receiverTouchPoints);
        scene()->sendEvent(originalReceiver, &tEvent);
        eaten = true;
    }

    // Unlock vertical movement after initial touch has been released
    // and modified touch event has been sent to receiver.
    if (hideOnNextMouseRelease) {
        extKeysArea->lockVerticalMovement(false);
    }

    return eaten;
}


bool ExtendedKeys::handleMouseEvent(QGraphicsSceneMouseEvent *event, EventOrigin from)
{
    bool eaten = false;
    QGraphicsSceneMouseEvent *generatedEvent = NULL;


    if (event->type() == QEvent::GraphicsSceneMousePress) {
        // Start following the first touch after initially
        // followed touch is released.
        if (followedTouchPointId < 0) {
            followedTouchPointId = 0;
            followedTouchPointOrigin = from;
            releasedTouchPointId = -1;
        } else if (followedTouchPointId != 0
                   || followedTouchPointOrigin != from) {
            // Ignore any new touch points until extended keys area
            // is hidden.
            ignoredTouchPoints[from].append(0);
        }
    }

    if (followedTouchPointId == 0
        && followedTouchPointOrigin == from) {
        generatedEvent = new QGraphicsSceneMouseEvent(event->type());
        generatedEvent->setPos(event->pos());
        generatedEvent->setScenePos(event->scenePos());
        generatedEvent->setLastPos(event->lastPos());
        generatedEvent->setLastScenePos(event->lastScenePos());
        eaten = true;

        if (event->type() == QEvent::GraphicsSceneMouseRelease) {
            // If multitouch is enabled, touch event that extKeysArea might be
            // interested in can come after mouse event. We want to redirect
            // that touch event to extKeysArea.
            releasedTouchPointId = followedTouchPointId;
            releasedTouchPointOrigin = followedTouchPointOrigin;
            followedTouchPointId = -1;

            if (!hideOnNextMouseRelease) {
                // Initial touch point is released
                hideOnNextMouseRelease = true;
            } else {
                // Second followed touch point is released, hide
                // extended keys area
                QMetaObject::invokeMethod(host, "hide", Qt::QueuedConnection);
            }
        }
    } else if (releasedTouchPointId == 0
               && releasedTouchPointOrigin == from) {
        // Release logic already handled at touch event handler.
        // Just redirect the mouse event to extKeyArea.
        releasedTouchPointId = -1;
        generatedEvent = new QGraphicsSceneMouseEvent(event->type());
        generatedEvent->setPos(event->pos());
        generatedEvent->setScenePos(event->scenePos());
        generatedEvent->setLastPos(event->lastPos());
        generatedEvent->setLastScenePos(event->lastScenePos());
        eaten = true;
    } else if (ignoredTouchPoints[from].contains(0)) {
        eaten = true;
    }

    if (event->type() == QEvent::GraphicsSceneMouseRelease
        && ignoredTouchPoints[from].contains(0)) {
        ignoredTouchPoints[from].removeOne(0);
    }

    if (generatedEvent) {
        if (showAnimation.state() != QAbstractAnimation::Stopped) {
            // Do not forward events to extKeysArea until animation has
            // finished. Instead, only update initial press position.
            tappedScenePos = generatedEvent->scenePos();
        } else {
            scene()->sendEvent(extKeysArea.get(), generatedEvent);
        }

        delete generatedEvent;
        generatedEvent = NULL;
    }

    // Unlock vertical movement after initial touch has been released
    // and modified mouse event has been sent to receiver.
    if (hideOnNextMouseRelease) {
        extKeysArea->lockVerticalMovement(false);
    }

    return eaten;
}

bool ExtendedKeys::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    bool eaten = false;

    // Mouse or touch event going to mainArea can be:
    // - Redirected to extKeysArea if:
    //   * touch point ID mathces initial followedTouchPointId
    //   * touch is first after initial touch has been released
    //     and extended keys area is still visible
    // - Passed to mainArea if the mouse or touch event isn't new
    // - Ignored if the mouse or touch event is new and not redirected
    if (event->type() == QEvent::TouchBegin
        || event->type() == QEvent::TouchUpdate
        || event->type() == QEvent::TouchEnd) {
        eaten = handleTouchEvent(static_cast<QTouchEvent*>(event), mainArea, MainKeyboardArea);
    } else if (event->type() == QEvent::GraphicsSceneMousePress
               || event->type() == QEvent::GraphicsSceneMouseRelease
               || event->type() == QEvent::GraphicsSceneMouseMove) {
        eaten = handleMouseEvent(static_cast<QGraphicsSceneMouseEvent*>(event), MainKeyboardArea);
    }

    return eaten;
}

bool ExtendedKeys::event(QEvent *event)
{
    bool eaten = false;

    // Mouse or touch event going to ExtendedKeys (MImOverlay) can be:
    // - Redirected to extKeysArea if:
    //   * touch is first after initial touch has been released
    //     and extended keys area is still visible
    // - Ignored if the mouse or touch event is not redirected
    if (event->type() == QEvent::TouchBegin
        || event->type() == QEvent::TouchUpdate
        || event->type() == QEvent::TouchEnd) {
        if (hideOnNextMouseRelease) {
            handleTouchEvent(static_cast<QTouchEvent*>(event), NULL, OverlayArea);
            eaten = true;
        }
    } else if (event->type() == QEvent::GraphicsSceneMousePress
               || event->type() == QEvent::GraphicsSceneMouseRelease
               || event->type() == QEvent::GraphicsSceneMouseMove) {
        if (hideOnNextMouseRelease) {
            handleMouseEvent(static_cast<QGraphicsSceneMouseEvent*>(event), OverlayArea);
            eaten = true;
        }
    }

    // Pass the event to parent class if not eaten here
    return eaten || MImOverlay::event(event);
}

qreal ExtendedKeys::magnitude() const
{
    return currentMagnitude;
}

void ExtendedKeys::setMagnitude(qreal value)
{
    if (value < 0.0f) {
        return;
    }

    currentMagnitude = value;
    QTransform t;
    t.translate(anchorPoint.x() * (1.0f - value),
                anchorPoint.y() * (1.0f - value));
    t.scale(value, value);
    extKeysArea->setTransform(t);
}

void ExtendedKeys::hideExtendedArea()
{
    showAnimation.stop();
    hide();
}

void ExtendedKeys::setAnchorPoint(const QPointF &anchor)
{
    anchorPoint = anchor;
}

void ExtendedKeys::addToGroup(QAnimationGroup *group)
{
    group->addAnimation(&showAnimation);
}

