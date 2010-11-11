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

#include "flickgesture.h"
#include "flickgesturerecognizer.h"
#include "mvirtualkeyboardstyle.h"
#include "mimabstractkeyarea.h"
#include "popupbase.h"
#include "popupfactory.h"

#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <MSceneManager>
#include <MGConfItem>
#include <QDebug>
#include <QEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QHash>
#include <QKeyEvent>
#include <mtimestamp.h>

namespace
{
    const qreal ZValueButtons = 0.0;

    // Minimal distinguishable cursor/finger movement
    const qreal MovementThreshold = 5.0;

    // For gesture thresholds: How many pixels translate to one counted move event.
    const qreal PixelsToMoveEventsFactor = 0.02;

    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";

    // Minimal distance (in px) for touch point from key button edge.
    const int CorrectionDistanceThreshold = 2;

    // Used for error correction:
    QPoint gAdjustedPositionForCorrection = QPoint();

    // Used for touchpoint conversion from mouse events:
    QPointF gLastMousePos = QPointF();

    bool isInsideAreaOf(const QPoint &target,
                        const QPoint &origin,
                        int xDistance,
                        int yDistance)
    {
        // Given target and origin, is target inside the rectangle of
        // (2 * xDistance) x (2 * yDistance), of which origin is the center?
        return ((target.x() > origin.x() - xDistance)
                && (target.x() < origin.x() + xDistance)
                && (target.y() > origin.y() - yDistance)
                && (target.y() < origin.y() + yDistance));
    }

    //! \brief Helper class responsible for finding active special keys.
    //!
    //! Can be used as visitor.
    class SpecialKeyFinder
            : public MImAbstractKeyVisitor
    {
    public:
        enum FindMode {
            FindShiftKey,
            FindDeadKey,
            FindBoth
        };

    private:
        MImAbstractKey *mShiftKey;
        MImAbstractKey *mDeadKey;
        FindMode mode;

    public:
        explicit SpecialKeyFinder(FindMode newMode = FindBoth)
            : MImAbstractKeyVisitor()
            , mShiftKey(0)
            , mDeadKey(0)
            , mode(newMode)
        {}

        MImAbstractKey *shiftKey() const
        {
            return mShiftKey;
        }

        MImAbstractKey *deadKey() const
        {
            return mDeadKey;
        }

        bool operator()(MImAbstractKey *key)
        {
            if (!key) {
                return false;
            }

            if (key->isShiftKey()) {
                mShiftKey = key;
            } else if (key->isDeadKey()) {
                mDeadKey = key;
            }

            switch (mode) {
            case FindShiftKey:
                if (mShiftKey) {
                    return true;
                }
                break;

            case FindDeadKey:
                if (mDeadKey) {
                    return true;
                }
                break;

            case FindBoth:
                if (mShiftKey && mDeadKey) {
                    return true;
                }
                break;
            }


            return false;
        }
    };
}

M::InputMethodMode MImAbstractKeyArea::InputMethodMode;

MImAbstractKeyArea::MImAbstractKeyArea(const LayoutData::SharedLayoutSection &newSection,
                                       bool usePopup,
                                       QGraphicsWidget *parent)
    : MStylableWidget(parent),
      mRelativeKeyBaseWidth(0),
      debugTouchPoints(style()->debugTouchPoints()),
      currentLevel(0),
      mPopup(usePopup ? PopupFactory::instance()->createPopup(this) : 0),
      wasGestureTriggered(false),
      enableMultiTouch(MGConfItem(MultitouchSettings).value().toBool()),
      feedbackPlayer(0),
      section(newSection)
{
    // By default multi-touch is disabled
    if (enableMultiTouch) {
        setAcceptTouchEvents(true);
    }

    lastTouchPointPressEvent.restart();
    grabGesture(FlickGestureRecognizer::sharedGestureType());
    feedbackPlayer = MComponentData::feedbackPlayer();

    longPressTimer.setSingleShot(true);
    idleVkbTimer.setSingleShot(true);

    connect(&longPressTimer, SIGNAL(timeout()),
            this, SLOT(handleLongKeyPressed()));

    connect(&idleVkbTimer, SIGNAL(timeout()),
            this, SLOT(handleIdleVkb()));

    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()),
            Qt::UniqueConnection);
}

MImAbstractKeyArea::~MImAbstractKeyArea()
{
    delete mPopup;
}

void MImAbstractKeyArea::setInputMethodMode(M::InputMethodMode inputMethodMode)
{
    InputMethodMode = inputMethodMode;
}

qreal MImAbstractKeyArea::relativeKeyBaseWidth() const
{
    return mRelativeKeyBaseWidth;
}

const LayoutData::SharedLayoutSection &MImAbstractKeyArea::sectionModel() const
{
    return section;
}

void MImAbstractKeyArea::updatePopup(MImAbstractKey *key)
{
    if (!mPopup) {
        return;
    }

    if (!key) {
        mPopup->cancel();
        return;
    }

    const QRectF &buttonRect = key->buttonRect();
    // IM FW guarantees that a scene position matches with
    // screen position, so we can use mapToScene to calculate screen position:
    const QPoint pos = mapToScene(buttonRect.topLeft()).toPoint();

    SpecialKeyFinder finder(SpecialKeyFinder::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&finder);

    mPopup->updatePos(buttonRect.topLeft(), pos, buttonRect.toRect().size());
    mPopup->handleKeyPressedOnMainArea(key,
                                       (finder.deadKey() ? finder.deadKey()->label() : QString()),
                                       level() % 2);
}

int MImAbstractKeyArea::maxColumns() const
{
    return section->maxColumns();
}

int MImAbstractKeyArea::rowCount() const
{
    return section->rowCount();
}

void
MImAbstractKeyArea::handleVisibilityChanged(bool visible)
{
    if (mPopup) {
        mPopup->setEnabled(visible);
    }

    if (!visible) {
        SpecialKeyFinder finder(SpecialKeyFinder::FindDeadKey);
        MImAbstractKey::visitActiveKeys(&finder);

        unlockDeadKeys(finder.deadKey());
    }
}

void
MImAbstractKeyArea::switchLevel(int level)
{
    if (level != currentLevel) {
        currentLevel = level;

        // Update uppercase / lowercase
        SpecialKeyFinder finder(SpecialKeyFinder::FindDeadKey);
        MImAbstractKey::visitActiveKeys(&finder);

        updateKeyModifiers(finder.deadKey() ? finder.deadKey()->label().at(0) : '\0');

        update();
    }
}

int MImAbstractKeyArea::level() const
{
    return currentLevel;
}

void MImAbstractKeyArea::setShiftState(ModifierState /*newShiftState*/)
{
    // Empty default implementation
}

void MImAbstractKeyArea::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    const int newWidth = static_cast<int>(event->newSize().width());

    if (newWidth != static_cast<int>(event->oldSize().width())) {
        updateKeyGeometries(newWidth);
    }
}

void MImAbstractKeyArea::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if (enableMultiTouch) {
        return;
    }

    touchPointPressed(createTouchPoint(0, Qt::TouchPointPressed,
                                       mapToScene(ev->pos()),
                                       mapToScene(gLastMousePos)));

    gLastMousePos = ev->pos();
}

void MImAbstractKeyArea::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    if (enableMultiTouch) {
        return;
    }

    touchPointMoved(createTouchPoint(0, Qt::TouchPointMoved,
                                     mapToScene(ev->pos()),
                                     mapToScene(gLastMousePos)));

    gLastMousePos = ev->pos();
}

void MImAbstractKeyArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    if (scene()->mouseGrabberItem() == this) {
        // Ungrab mouse explicitly since we probably used grabMouse() to get it.
        ungrabMouse();
    }

    if (enableMultiTouch) {
        return;
    }

    touchPointReleased(createTouchPoint(0, Qt::TouchPointReleased,
                                        mapToScene(ev->pos()),
                                        mapToScene(gLastMousePos)));

    gLastMousePos = ev->pos();
}

void MImAbstractKeyArea::click(MImAbstractKey *key,
                               const QPoint &pos)
{
    if (!key) {
        return;
    }

    SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    if (!key->isDeadKey()) {
        const QString accent = (finder.deadKey() ? finder.deadKey()->label() : QString());
        emit keyClicked(key, accent, hasActiveShiftKeys || level() % 2, pos);

        if (!key->isShiftKey()) {
            unlockDeadKeys(finder.deadKey());
        }
    } else if (key == finder.deadKey()) {
        unlockDeadKeys(finder.deadKey());
    } else {
        // Deselect previous dead key, if any:
        if (finder.deadKey()) {
            finder.deadKey()->setSelected(false);
        }

        // key is the new deadkey:
        key->setSelected(true);
        updateKeyModifiers(key->label().at(0));
    }
}

QVariant MImAbstractKeyArea::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemVisibleChange:
        handleVisibilityChanged(value.toBool());
        break;

    default:
        break;
    }

    return QGraphicsItem::itemChange(change, value);
}

void MImAbstractKeyArea::grabMouseEvent(QEvent */*event*/)
{
    // If mimabstractkeyarea is hidden without mouseReleaseEvent
    // the enabled <flicked> would stay true if mouse
    // grab is obtained again without mousePressEvent.
    // This would ignore mouseReleaseEvent and would not cause keyClicked.
    wasGestureTriggered = false;

    const qreal ScalingFactor = style()->flickGestureThresholdRatio();
    const int HorizontalThreshold = static_cast<int>(boundingRect().width() * ScalingFactor);
    const int VerticalThreshold = static_cast<int>(boundingRect().height() * ScalingFactor);
    const int Timeout = style()->flickGestureTimeout();

    FlickGestureRecognizer::instance()->setFinishThreshold(HorizontalThreshold, VerticalThreshold);
    FlickGestureRecognizer::instance()->setStartThreshold(HorizontalThreshold / 2, VerticalThreshold / 2);
    FlickGestureRecognizer::instance()->setTimeout(Timeout);
}

void MImAbstractKeyArea::ungrabMouseEvent(QEvent *)
{
    // Make sure popup can respond to mouse grab removal:
    if (mPopup) {
        mPopup->cancel();
    }

    longPressTimer.stop();
}

bool MImAbstractKeyArea::event(QEvent *ev)
{
    bool eaten = false;

    if (ev->type() == QEvent::Gesture) {
        const Qt::GestureType flickGestureType = FlickGestureRecognizer::sharedGestureType();
        FlickGesture *flickGesture = static_cast<FlickGesture *>(static_cast<QGestureEvent *>(ev)->gesture(flickGestureType));

        if (flickGesture) {
            handleFlickGesture(flickGesture);
            eaten = true;
        }
    } else if (ev->type() == QEvent::TouchBegin
               || ev->type() == QEvent::TouchUpdate
               || ev->type() == QEvent::TouchEnd) {
        QTouchEvent *touch = static_cast<QTouchEvent*>(ev);

        foreach (const QTouchEvent::TouchPoint &tp, touch->touchPoints()) {

            switch (tp.state()) {
            case Qt::TouchPointPressed:
                touchPointPressed(tp);
                break;

            case Qt::TouchPointMoved:
                touchPointMoved(tp);
                break;

            case Qt::TouchPointReleased:
                touchPointReleased(tp);
                break;
            default:
                break;
            }
        }

        eaten = true;
    }

    return eaten || MWidget::event(ev);
}

void MImAbstractKeyArea::handleFlickGesture(FlickGesture *gesture)
{
    if (InputMethodMode == M::InputMethodModeDirect) {
        return;
    }

    // Any flick gesture, complete or not, resets active keys etc.
    if (!wasGestureTriggered && (gesture->state() != Qt::NoGesture)) {

        if (mPopup) {
            mPopup->cancel();
        }

        if (MImAbstractKey *key = MImAbstractKey::lastActiveKey()) {
            key->resetTouchPointCount();
        }

        longPressTimer.stop();
        wasGestureTriggered = true;
    }

    if (gesture->state() == Qt::GestureFinished) {

        switch (gesture->direction()) {
        case FlickGesture::Left:
            emit flickLeft();
            break;

        case FlickGesture::Right:
            emit flickRight();
            break;

        case FlickGesture::Down:
            emit flickDown();
            break;

        case FlickGesture::Up: {
                const MImAbstractKey *flickedKey = keyAt(gesture->startPosition());
                if (flickedKey) {
                    emit flickUp(flickedKey->binding());
                }
                break;
            }

        default:
            return;
        }
    }
}

void MImAbstractKeyArea::touchPointPressed(const QTouchEvent::TouchPoint &tp)
{
    wasGestureTriggered = false;

    // Gestures only slow down in speed typing mode:
    if (isInSpeedTypingMode(true)) {
        idleVkbTimer.stop();
        // TODO: check how expensive gesture (un)grabbing is:
        ungrabGesture(FlickGestureRecognizer::sharedGestureType());
    }

    const QPoint pos = mapFromScene(tp.scenePos()).toPoint();
    MImAbstractKey *key = keyAt(pos);

    if (debugTouchPoints) {
        printTouchPoint(tp, key);
    }

    if (!key) {
        longPressTimer.stop();
        return;
    }

    // Try to commit currently active key before activating new key:
    MImAbstractKey *const lastActiveKey = MImAbstractKey::lastActiveKey();
    SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    if (lastActiveKey
        && lastActiveKey->isNormalKey()
        && lastActiveKey->touchPointCount() > 0) {
        // TODO: play release sound? Potentially confusing to user, who
        // might still press this key.
        emit keyClicked(lastActiveKey, QString(),
                        hasActiveShiftKeys || level() % 2,
                        gAdjustedPositionForCorrection);

        lastActiveKey->resetTouchPointCount();
    }

    if (key->increaseTouchPointCount()
        && key->touchPointCount() == 1) {
        updatePopup(key);
        longPressTimer.start(style()->longPressTimeout());

        emit keyPressed(key, (finder.deadKey() ? finder.deadKey()->label() : QString()),
                        hasActiveShiftKeys || level() % 2);
    }
}

void MImAbstractKeyArea::touchPointMoved(const QTouchEvent::TouchPoint &tp)
{
    if (wasGestureTriggered) {
        longPressTimer.stop();
        return;
    }

    const QPoint pos = mapFromScene(tp.scenePos()).toPoint();
    const QPoint lastPos = mapFromScene(tp.lastScenePos()).toPoint();
    const QPoint startPos = mapFromScene(tp.startScenePos()).toPoint();

    const GravitationalLookupResult lookup = gravitationalKeyAt(pos, lastPos, startPos);
    SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    // For a moving touchpoint, we only need to consider enter-key or leave-key events:
    if (lookup.key != lookup.lastKey) {

        if (lookup.key
            && lookup.key->increaseTouchPointCount()
            && lookup.key->touchPointCount() == 1) {
            // Reaction map cannot discover when we move from one key
            // (= reactive area) to another
            feedbackPlayer->play(MFeedbackPlayer::Press);
            longPressTimer.start(style()->longPressTimeout());
            emit keyPressed(lookup.key,
                            (finder.deadKey() ? finder.deadKey()->label() : QString()),
                            hasActiveShiftKeys || level() % 2);
        }

        if (lookup.lastKey
            && lookup.lastKey->decreaseTouchPointCount()
            && lookup.lastKey->touchPointCount() == 0) {
            // Reaction map cannot discover when we move from one key
            // (= reactive area) to another
            feedbackPlayer->play(MFeedbackPlayer::Cancel);
            emit keyReleased(lookup.lastKey,
                             (finder.deadKey() ? finder.deadKey()->label() : QString()),
                             hasActiveShiftKeys || level() % 2);
        }
    }

    if (!lookup.key) {
        longPressTimer.stop();
    } else {
        updatePopup(lookup.key);
    }

    if (debugTouchPoints) {
        printTouchPoint(tp, lookup.key, lookup.lastKey);
    }
}

void MImAbstractKeyArea::touchPointReleased(const QTouchEvent::TouchPoint &tp)
{
    if (wasGestureTriggered) {
        return;
    }

    idleVkbTimer.start(style()->idleVkbTimeout());

    const QPoint pos = mapFromScene(tp.scenePos()).toPoint();
    const QPoint lastPos = mapFromScene(tp.lastScenePos()).toPoint();
    const QPoint startPos = mapFromScene(tp.startScenePos()).toPoint();

    const GravitationalLookupResult lookup = gravitationalKeyAt(pos, lastPos, startPos);
    SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    if (lookup.key
        && lookup.key->decreaseTouchPointCount()
        && lookup.key->touchPointCount() == 0) {
        longPressTimer.stop();
        emit keyReleased(lookup.key,
                         (finder.deadKey() ? finder.deadKey()->label() : QString()),
                         hasActiveShiftKeys || level() % 2);

        click(lookup.key, gAdjustedPositionForCorrection);
    }

    if (lookup.lastKey
        && lookup.lastKey != lookup.key
        && lookup.lastKey->decreaseTouchPointCount()
        && lookup.lastKey->touchPointCount() == 0) {
        emit keyReleased(lookup.lastKey,
                         (finder.deadKey() ? finder.deadKey()->label() : QString()),
                         hasActiveShiftKeys || level() % 2);
    }

    // We're finished with this touch point, inform popup:
    if (mPopup) {
        mPopup->cancel();
    }

    longPressTimer.stop();

    if (debugTouchPoints) {
        printTouchPoint(tp, lookup.key, lookup.lastKey);
    }
}

QTouchEvent::TouchPoint MImAbstractKeyArea::createTouchPoint(int id,
                                                             Qt::TouchPointState state,
                                                             const QPointF &pos,
                                                             const QPointF &lastPos)
{
    QTouchEvent::TouchPoint tp(id);
    tp.setState(state);
    tp.setScenePos(pos);
    tp.setLastScenePos(lastPos);

    return tp;
}

MImAbstractKeyArea::GravitationalLookupResult
MImAbstractKeyArea::gravitationalKeyAt(const QPoint &pos,
                                  const QPoint &lastPos,
                                  const QPoint &startPos) const
{
    // TODO: Needs explicit test coverage, maybe.
    MImAbstractKey *key = 0;
    MImAbstractKey *lastKey = 0;

    const qreal hGravity = style()->touchpointHorizontalGravity();
    const qreal vGravity = style()->touchpointVerticalGravity();

    key = keyAt(isInsideAreaOf(pos, startPos, hGravity, vGravity)
                ? startPos : pos);

    lastKey = keyAt(isInsideAreaOf(lastPos, startPos, hGravity, vGravity)
                    ? startPos : lastPos);

    // Check whether error correction needs updated position:
    if (key) {
        QPoint &ec = gAdjustedPositionForCorrection;
        const QRectF &br = key->buttonRect();

        ec.setX(qBound<int>(br.left() + CorrectionDistanceThreshold,
                            pos.x(),
                            br.right() - CorrectionDistanceThreshold));

        ec.setY(qBound<int>(br.top() + CorrectionDistanceThreshold,
                            pos.y(),
                            br.bottom() - CorrectionDistanceThreshold));
    }

    return GravitationalLookupResult(key, lastKey);
}

void MImAbstractKeyArea::unlockDeadKeys(MImAbstractKey *deadKey)
{
    if (!deadKey || !deadKey->isDeadKey()) {
        return;
    }

    deadKey->setSelected(false);
    deadKey->resetTouchPointCount();
    updateKeyModifiers('\0');
}

void MImAbstractKeyArea::drawReactiveAreas(MReactionMap *,
                                           QGraphicsView *)
{
    // Empty default implementation. Geometries of buttons are known by derived classes.
}

const PopupBase &MImAbstractKeyArea::popup() const
{
    return *mPopup;
}

void MImAbstractKeyArea::printTouchPoint(const QTouchEvent::TouchPoint &tp,
                                    const MImAbstractKey *key,
                                    const MImAbstractKey *lastKey) const
{
    // Sorry that this looks a bit funny ... it does the job, though.
    qDebug() << "\ntouchpoint:" << tp.id() << "(start:" << tp.startScenePos()
                                << ", last:" << tp.lastScenePos()
                                << ", pos:"  << tp.scenePos() << ")\n      "
             << "key:" << key
                       << "(label:" << (key ? QString("%1: %2").arg(key->label())
                                                               .arg(key->state())
                                            : QString()) << ")\n "
             << "last key:" << lastKey
                            << "(label:" << (lastKey ? QString("%1: %2").arg(lastKey->label())
                                                                        .arg(lastKey->state())
                                                     : QString()) << ")";
}

void MImAbstractKeyArea::updateKeyModifiers(const QChar &accent)
{
    // We currently don't allow active dead key level changing. If we did,
    // we should update activeDeadKey level before delivering its accent to
    // other keys.
    bool shift = (currentLevel == 1);
    modifiersChanged(shift, accent);
}

void MImAbstractKeyArea::modifiersChanged(bool,
                                          const QChar &)
{
    // Empty default implementation
}

void MImAbstractKeyArea::onThemeChangeCompleted()
{
    // TODO: update all other CSS attributes that are mapped to members.
    updateKeyGeometries(size().width());
}

const MImAbstractKeyAreaStyleContainer &MImAbstractKeyArea::baseStyle() const
{
    return style();
}

void MImAbstractKeyArea::handleLongKeyPressed()
{
    MImAbstractKey *const lastActiveKey = MImAbstractKey::lastActiveKey();

    if (!lastActiveKey) {
        return;
    }

    SpecialKeyFinder finder(SpecialKeyFinder::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&finder);

    const QString accent = (finder.deadKey() ? finder.deadKey()->label()
                                           : QString());

    if (mPopup) {
        mPopup->handleLongKeyPressedOnMainArea(lastActiveKey, accent, level() % 2);
    }

    emit longKeyPressed(lastActiveKey, accent, level() % 2);
}

void MImAbstractKeyArea::handleIdleVkb()
{
    grabGesture(FlickGestureRecognizer::sharedGestureType());
}

bool MImAbstractKeyArea::isInSpeedTypingMode(bool restartTimers)
{
    return ((restartTimers ? lastTouchPointPressEvent.restart()
                           : lastTouchPointPressEvent.elapsed()) < style()->idleVkbTimeout());
}
