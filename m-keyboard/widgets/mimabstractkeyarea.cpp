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
#include "mimabstractkeyarea.h"
#include "mimkeyvisitor.h"
#include "popupbase.h"
#include "popupfactory.h"
#include "mkeyboardhost.h"

#include <MFeedback>
#include <MGConfItem>
#include <QDebug>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <mtimestamp.h>

#ifdef unix
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char *const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";

    // Minimal distance (in px) for touch point from key button edge.
    const int CorrectionDistanceThreshold = 2;

    // Used for error correction:
    QPoint gAdjustedPositionForCorrection = QPoint();

    // Used for touchpoint conversion from mouse events:
    QPointF gLastMousePos = QPointF();

    QPointF adjustedByGravity(const MImAbstractKey *const key,
                              const QPoint &pos,
                              int horizontalGravity,
                              int verticalGravity)
    {
        if (key
            && key->isGravityActive()
            && key->buttonBoundingRect().adjusted(-horizontalGravity, -verticalGravity,
                                                   horizontalGravity,  verticalGravity).contains(pos)) {
            return key->buttonBoundingRect().center();
        }

        return pos;
    }

    QString toString(const QPointF &p, const QString &separator = ", ")
    {
        return QString("%1%2%3").arg(p.x())
                                .arg(separator)
                                .arg(p.y());
    }

    QString toString(const QRectF &r, const QString separator = ", ")
    {
        return QString("%1%2%3%4%5").arg(toString(r.topLeft(), separator))
                                    .arg(separator)
                                    .arg(r.width())
                                    .arg(separator)
                                    .arg(r.height());
    }

    QString toString(Qt::TouchPointState state)
    {
        switch(state) {
        default:
            return "n/a";

        case Qt::TouchPointPressed:
            return "pressed";

        case Qt::TouchPointMoved:
            return "moved";

        case Qt::TouchPointStationary:
            return "stationary";

        case Qt::TouchPointReleased:
            return "released";
        }
    }

    QString timeStamp()
    {
#ifdef unix
        struct timeval tv;
        gettimeofday(&tv, 0);
        return  QString("%1.%2").arg(tv.tv_sec).arg(tv.tv_usec);
#else
        return QTime::currentTime().toString();
#endif
    }
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
      feedbackPress(MFeedback::Press),
      feedbackCancel(MFeedback::Cancel),
      section(newSection)
{
    // By default multi-touch is disabled
    if (enableMultiTouch) {
        setAcceptTouchEvents(true);
    }

    lastTouchPointPressEvent.restart();
    grabGesture(FlickGestureRecognizer::sharedGestureType());

    longPressTimer.setSingleShot(true);
    idleVkbTimer.setSingleShot(true);

    connect(&longPressTimer, SIGNAL(timeout()),
            this, SLOT(handleLongKeyPressed()));

    connect(&idleVkbTimer, SIGNAL(timeout()),
            this, SLOT(handleIdleVkb()));

    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()),
            Qt::UniqueConnection);

    switchStyleMode();
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

    MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::SpecialKeyFinder::FindDeadKey);
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
    if (!visible) {
        if (mPopup) {
            mPopup->setVisible(false);
        }

        MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::SpecialKeyFinder::FindDeadKey);
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
        MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::SpecialKeyFinder::FindDeadKey);
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

    MImKeyVisitor::SpecialKeyFinder finder;
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

    if (enableMultiTouch && MImAbstractKey::lastActiveKey()) {
        MImAbstractKey::lastActiveKey()->resetTouchPointCount();
    }

    longPressTimer.stop();
}

bool MImAbstractKeyArea::event(QEvent *ev)
{
    bool eaten = false;
    QString start, end;
    start = QString("%1|start").arg(ev->type());
    end = QString("%1|end").arg(ev->type());
    mTimestamp("MImAbstractKeyArea", start);

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

    const bool result = eaten || MWidget::event(ev);

    mTimestamp("MImAbstractKeyArea", end);
    return result;
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
    mTimestamp("MImAbstractKeyArea", "start");
    wasGestureTriggered = false;

    // Gestures only slow down in speed typing mode:
    if (isInSpeedTypingMode(true)) {
        idleVkbTimer.stop();
        // TODO: check how expensive gesture (un)grabbing is:
        ungrabGesture(FlickGestureRecognizer::sharedGestureType());
    }

    const QPoint pos = correctedTouchPoint(tp.scenePos());
    MImAbstractKey *key = keyAt(pos);


    if (debugTouchPoints) {
        logTouchPoint(tp, key);
    }

    if (!key) {
        longPressTimer.stop();
        mTimestamp("MImAbstractKeyArea", "end");
        return;
    }

    // Try to commit currently active key before activating new key:
    MImAbstractKey *const lastActiveKey = MImAbstractKey::lastActiveKey();
    MImKeyVisitor::SpecialKeyFinder finder;
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

        // We activate the key's gravity here because a key cannot
        // differentiate between initially-pressed-key or
        // activated-by-moving-onto-it.
        // However, the key deactivates the gravity itself again
        // (touchpoint count goes from 1 to 0).
        key->activateGravity();

        emit keyPressed(key, (finder.deadKey() ? finder.deadKey()->label() : QString()),
                        hasActiveShiftKeys || level() % 2);
    }
    mTimestamp("MImAbstractKeyArea", "end");
}

void MImAbstractKeyArea::touchPointMoved(const QTouchEvent::TouchPoint &tp)
{
    if (wasGestureTriggered) {
        longPressTimer.stop();
        return;
    }

    if (tp.scenePos() == tp.lastScenePos()) {
        return;
    }

    mTimestamp("MImAbstractKeyArea", "start");

    const QPoint pos = correctedTouchPoint(tp.scenePos());
    const QPoint lastPos = correctedTouchPoint(tp.lastScenePos());

    const GravitationalLookupResult lookup = gravitationalKeyAt(pos, lastPos);
    MImKeyVisitor::SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    // For a moving touchpoint, we only need to consider enter-key or leave-key events:
    if (lookup.key != lookup.lastKey) {

        if (lookup.key) {
            updatePopup(lookup.key);

            if (lookup.key->increaseTouchPointCount()
                && lookup.key->touchPointCount() == 1) {
                // Reaction map cannot discover when we move from one key
                // (= reactive area) to another
                // slot is called asynchronously to get screen update as fast as possible
                QMetaObject::invokeMethod(&feedbackPress, "play", Qt::QueuedConnection);
                longPressTimer.start(style()->longPressTimeout());
                emit keyPressed(lookup.key,
                                (finder.deadKey() ? finder.deadKey()->label() : QString()),
                                hasActiveShiftKeys || level() % 2);
            }
        }

        if (lookup.lastKey
            && lookup.lastKey->decreaseTouchPointCount()
            && lookup.lastKey->touchPointCount() == 0) {
            // Reaction map cannot discover when we move from one key
            // (= reactive area) to another
            // slot is called asynchronously to get screen update as fast as possible
            QMetaObject::invokeMethod(&feedbackCancel, "play", Qt::QueuedConnection);
            emit keyReleased(lookup.lastKey,
                             (finder.deadKey() ? finder.deadKey()->label() : QString()),
                             hasActiveShiftKeys || level() % 2);
        }
    }

    if (!lookup.key) {
        longPressTimer.stop();
    }

    if (debugTouchPoints) {
        logTouchPoint(tp, lookup.key, lookup.lastKey);
    }
    mTimestamp("MImAbstractKeyArea", "end");
}

void MImAbstractKeyArea::touchPointReleased(const QTouchEvent::TouchPoint &tp)
{
    if (wasGestureTriggered) {
        return;
    }
    mTimestamp("MImAbstractKeyArea", "start");

    idleVkbTimer.start(style()->idleVkbTimeout());

    const QPoint pos = correctedTouchPoint(tp.scenePos());
    const QPoint lastPos = correctedTouchPoint(tp.lastScenePos());

    const GravitationalLookupResult lookup = gravitationalKeyAt(pos, lastPos);
    MImKeyVisitor::SpecialKeyFinder finder;
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
        logTouchPoint(tp, lookup.key, lookup.lastKey);
    }
    mTimestamp("MImAbstractKeyArea", "end");
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
                                       const QPoint &lastPos) const
{
    // TODO: Needs explicit test coverage, maybe.
    MImAbstractKey *key = 0;
    MImAbstractKey *lastKey = 0;

    const qreal hGravity = style()->touchpointHorizontalGravity();
    const qreal vGravity = style()->touchpointVerticalGravity();

    key = keyAt(adjustedByGravity(MImAbstractKey::lastActiveKey(),
                                  pos, hGravity, vGravity).toPoint());
    lastKey = keyAt(adjustedByGravity(MImAbstractKey::lastActiveKey(),
                                      lastPos, hGravity, vGravity).toPoint());

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

void MImAbstractKeyArea::hidePopup()
{
    if (!mPopup) {
        return;
    }

    mPopup->setVisible(false);
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

void MImAbstractKeyArea::logTouchPoint(const QTouchEvent::TouchPoint &tp,
                                       const MImAbstractKey *key,
                                       const MImAbstractKey *lastKey) const
{
    static bool headerWritten = false;
    QTextStream &out = MKeyboardHost::instance()->touchPointLog();

    if (!headerWritten) {
        out << "time (sec.msec)\t"
            << "tp_id\t"
            << "tp_state\t"
            << "start_x\t" << "start_y\t"
            << "last_x\t" << "last_y\t"
            << "current_x\t" << "current_y\t"
            << "center_x\t" << "center_y\t"
            << "delta_x\t" << "delta_y\t"
            << "label\t"
            << "label_last\t"
            << "br_x\t" << "br_y\t" << "br_w\t" << "br_h\n";
        headerWritten = true;
    }

    out << timeStamp() << "\t"
        << tp.id() << "\t"
        << toString(tp.state()) << "\t"
        << toString(tp.startScenePos(), "\t") << "\t"
        << toString(tp.lastScenePos(), "\t") << "\t"
        << toString(tp.scenePos(), "\t") << "\t"
        << (key ? toString(mapRectToScene(key->buttonBoundingRect()).center(), "\t")
                : "n/a\t") << "\t"
        << (key ? toString(tp.scenePos() - mapRectToScene(key->buttonBoundingRect()).center(), "\t")
                : "n/a\t") << "\t"
        << (key ? key->label() : "n/a") << "\t"
        << (lastKey ? lastKey->label() : "n/a") << "\t"
        << (key ? toString(mapRectToScene(key->buttonBoundingRect()), "\t")
                : "n/a\t\t\t") << "\n";
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
    switchStyleMode();
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

    MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::SpecialKeyFinder::FindDeadKey);
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

void MImAbstractKeyArea::switchStyleMode()
{
    if (!style()->syncStyleModeWithKeyCount()) {
        return;
    }

    switch(section->keyCount()) {

    case 13:
        style().setModeKeys13();
        break;

    case 30:
        style().setModeKeys30();
        break;

    case 31:
        style().setModeKeys31();
        break;

    case 32:
        style().setModeKeys32();
        break;

    case 33:
        style().setModeKeys33();
        break;

    case 34:
        style().setModeKeys34();
        break;

    case 35:
        style().setModeKeys35();
        break;

    case 36:
        style().setModeKeys36();
        break;

    case 37:
        style().setModeKeys37();
        break;

    case 38:
        style().setModeKeys38();
        break;

    case 39:
        style().setModeKeys39();
        break;

    case 40:
        style().setModeKeys40();
        break;

    case 41:
        style().setModeKeys41();
        break;

    case 42:
        style().setModeKeys42();
        break;

    case 43:
        style().setModeKeys43();
        break;

    case 44:
        style().setModeKeys44();
        break;

    case 45:
        style().setModeKeys45();
        break;

    default:
        break;
    }
}

void MImAbstractKeyArea::reset(bool resetCapsLock)
{
    if (scene()->mouseGrabberItem() == this) {
        // Ungrab mouse explicitly since we probably used grabMouse() to get it.
        ungrabMouse();
    }

    if (mPopup) {
        mPopup->cancel();
    }

    if (resetCapsLock) {
        MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::SpecialKeyFinder::FindShiftKey);
        MImAbstractKey::visitActiveKeys(&finder);
        bool hasCapsLocked =  finder.shiftKey() ? (finder.shiftKey()->state() == MImAbstractKey::Selected) : false;
        MImAbstractKey::resetActiveKeys();
        modifiersChanged(hasCapsLocked);
    } else {
        // release active keys (whilst preserving caps-lock)
        MImKeyVisitor::KeyAreaReset reset;
        MImAbstractKey::visitActiveKeys(&reset);
        modifiersChanged(reset.hasCapsLocked());
    }
    update();
}

QPoint MImAbstractKeyArea::correctedTouchPoint(const QPointF &scenePos) const
{
    QPointF pos = mapFromScene(scenePos);

    if (pos.y() >= baseStyle()->touchpointVerticalOffset()) {
        pos.ry() -= baseStyle()->touchpointVerticalOffset();
    }

    return pos.toPoint();
}

QRectF MImAbstractKeyArea::correctedReactionRect(const QRectF &originalRect) const
{
    QRectF rect = originalRect;
    float offset = baseStyle()->touchpointVerticalOffset();

    if (rect.top() >= offset) {
        rect.setTop(rect.top() + offset);
    }
    rect.setBottom(rect.bottom() + offset);

    return rect;
}
