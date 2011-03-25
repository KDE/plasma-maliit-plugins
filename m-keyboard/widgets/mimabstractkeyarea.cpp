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

#include "flickgesture.h"
#include "flickgesturerecognizer.h"
#include "mimabstractkeyarea.h"
#include "mimabstractkeyarea_p.h"
#include "mimkeyvisitor.h"
#include "mimreactionmap.h"
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

M::InputMethodMode MImAbstractKeyAreaPrivate::InputMethodMode;

MImAbstractKeyAreaPrivate::MImAbstractKeyAreaPrivate(const LayoutData::SharedLayoutSection &newSection,
                                                     MImAbstractKeyArea *owner)
    : q_ptr(owner),
      currentLevel(0),
      wasGestureTriggered(false),
      enableMultiTouch(MGConfItem(MultitouchSettings).value().toBool()),
      feedbackSliding(MImReactionMap::Sliding),
      section(newSection),
      allowedHorizontalFlick(true)
{
}

MImAbstractKeyAreaPrivate::~MImAbstractKeyAreaPrivate()
{
    delete mPopup;
}

void MImAbstractKeyAreaPrivate::click(MImAbstractKey *key,
                                      const QPoint &pos)
{
    Q_Q(MImAbstractKeyArea);

    if (!key || !key->enabled()) {
        return;
    }

    MImKeyVisitor::SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    if (!key->isDeadKey()) {
        const QString accent = (finder.deadKey() ? finder.deadKey()->label() : QString());
        emit q->keyClicked(key, accent, hasActiveShiftKeys || isUpperCase(), pos);

        if (!key->isShiftKey()) {
            q->unlockDeadKeys(finder.deadKey());
        }
    } else if (key == finder.deadKey()) {
        q->unlockDeadKeys(finder.deadKey());
    } else {
        // Deselect previous dead key, if any:
        if (finder.deadKey()) {
            finder.deadKey()->setSelected(false);
        }

        // key is the new deadkey:
        key->setSelected(true);
        q->updateKeyModifiers(key->label().at(0));
    }
}

void MImAbstractKeyAreaPrivate::handleFlickGesture(FlickGesture *gesture)
{
    Q_Q(MImAbstractKeyArea);

    if (InputMethodMode == M::InputMethodModeDirect) {
        return;
    }

    if (!allowedHorizontalFlick) {
        if ((gesture->direction() == FlickGesture::Left)
            || (gesture->direction() == FlickGesture::Right)) {
            return;
        }
    }

    // Any flick gesture, complete or not, resets active keys etc.
    if (!wasGestureTriggered && (gesture->state() != Qt::NoGesture)) {

        if (mPopup) {
            mPopup->cancel();
        }

        MImKeyVisitor::KeyAreaReset reset;
        MImAbstractKey::visitActiveKeys(&reset);

        longPressTimer.stop();
        wasGestureTriggered = true;
    }

    if (gesture->state() == Qt::GestureFinished) {

        switch (gesture->direction()) {
        case FlickGesture::Left:
            emit q->flickLeft();
            break;

        case FlickGesture::Right:
            emit q->flickRight();
            break;

        case FlickGesture::Down:
            emit q->flickDown();
            break;

        case FlickGesture::Up: {
                const MImAbstractKey *flickedKey = q->keyAt(gesture->startPosition());
                if (flickedKey) {
                    emit q->flickUp(flickedKey->binding());
                }
                break;
            }

        default:
            return;
        }
    }
}

void MImAbstractKeyAreaPrivate::touchPointPressed(const QTouchEvent::TouchPoint &tp)
{
    Q_Q(MImAbstractKeyArea);

    mTimestamp("MImAbstractKeyArea", "start");
    wasGestureTriggered = false;

    // Gestures only slow down in speed typing mode:
    if (isInSpeedTypingMode(true)) {
        idleVkbTimer.stop();
        // TODO: check how expensive gesture (un)grabbing is:
        q->ungrabGesture(FlickGestureRecognizer::sharedGestureType());
    }

    const QPoint pos = q->correctedTouchPoint(tp.scenePos());
    MImAbstractKey *key = q->keyAt(pos);


    if (q->debugTouchPoints) {
        q->logTouchPoint(tp, key);
    }

    if (!key || !key->enabled()) {
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
        && lastActiveKey->enabled()
        && lastActiveKey->isNormalKey()
        && lastActiveKey->touchPointCount() > 0) {
        // TODO: play release sound? Potentially confusing to user, who
        // might still press this key.
        emit q->keyClicked(lastActiveKey, QString(),
                           hasActiveShiftKeys || isUpperCase(),
                           gAdjustedPositionForCorrection);

        lastActiveKey->resetTouchPointCount();
    }

    if (key->increaseTouchPointCount()
        && key->touchPointCount() == 1) {
        q->updatePopup(key);
        longPressTimer.start(q->style()->longPressTimeout());

        // We activate the key's gravity here because a key cannot
        // differentiate between initially-pressed-key or
        // activated-by-moving-onto-it.
        // However, the key deactivates the gravity itself again
        // (touchpoint count goes from 1 to 0).
        key->activateGravity();

        emit q->keyPressed(key, (finder.deadKey() ? finder.deadKey()->label() : QString()),
                           hasActiveShiftKeys || isUpperCase());
    }
    mTimestamp("MImAbstractKeyArea", "end");
}

void MImAbstractKeyAreaPrivate::touchPointMoved(const QTouchEvent::TouchPoint &tp)
{
    Q_Q(MImAbstractKeyArea);

    if (wasGestureTriggered) {
        longPressTimer.stop();
        return;
    }

    if (tp.scenePos() == tp.lastScenePos()) {
        return;
    }

    mTimestamp("MImAbstractKeyArea", "start");

    const QPoint pos = q->correctedTouchPoint(tp.scenePos());
    const QPoint lastPos = q->correctedTouchPoint(tp.lastScenePos());

    const GravitationalLookupResult lookup = gravitationalKeyAt(pos, lastPos);
    MImKeyVisitor::SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    // For a moving touchpoint, we only need to consider enter-key or leave-key events:
    if (lookup.key != lookup.lastKey) {

        if (lookup.key) {
            q->updatePopup(lookup.key);

            if (lookup.key->enabled()
                && lookup.key->increaseTouchPointCount()
                && lookup.key->touchPointCount() == 1) {
                // Reaction map cannot discover when we move from one key
                // (= reactive area) to another
                // slot is called asynchronously to get screen update as fast as possible
                QMetaObject::invokeMethod(&feedbackSliding, "play", Qt::QueuedConnection);
                longPressTimer.start(q->style()->longPressTimeout());
                emit q->keyPressed(lookup.key,
                                   (finder.deadKey() ? finder.deadKey()->label() : QString()),
                                   hasActiveShiftKeys || isUpperCase());
            }
        }

        if (lookup.lastKey
            && lookup.lastKey->enabled()
            && lookup.lastKey->decreaseTouchPointCount()
            && lookup.lastKey->touchPointCount() == 0) {
            emit q->keyReleased(lookup.lastKey,
                                (finder.deadKey() ? finder.deadKey()->label() : QString()),
                                hasActiveShiftKeys || isUpperCase());
        }
    }

    if (!lookup.key || !lookup.key->enabled()) {
        longPressTimer.stop();
    }

    if (q->debugTouchPoints) {
        q->logTouchPoint(tp, lookup.key, lookup.lastKey);
    }
    mTimestamp("MImAbstractKeyArea", "end");
}

void MImAbstractKeyAreaPrivate::touchPointReleased(const QTouchEvent::TouchPoint &tp)
{
    Q_Q(MImAbstractKeyArea);

    if (wasGestureTriggered) {
        return;
    }
    mTimestamp("MImAbstractKeyArea", "start");

    idleVkbTimer.start(q->style()->idleVkbTimeout());

    const QPoint pos = q->correctedTouchPoint(tp.scenePos());
    const QPoint lastPos = q->correctedTouchPoint(tp.lastScenePos());

    const GravitationalLookupResult lookup = gravitationalKeyAt(pos, lastPos);
    MImKeyVisitor::SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    if (lookup.key
        && lookup.key->enabled()
        && lookup.key->decreaseTouchPointCount()
        && lookup.key->touchPointCount() == 0) {
        longPressTimer.stop();
        emit q->keyReleased(lookup.key,
                            (finder.deadKey() ? finder.deadKey()->label() : QString()),
                            hasActiveShiftKeys || isUpperCase());

        click(lookup.key, gAdjustedPositionForCorrection);
    }

    if (lookup.lastKey
        && lookup.lastKey != lookup.key
        && lookup.lastKey->enabled()
        && lookup.lastKey->decreaseTouchPointCount()
        && lookup.lastKey->touchPointCount() == 0) {
        emit q->keyReleased(lookup.lastKey,
                            (finder.deadKey() ? finder.deadKey()->label() : QString()),
                            hasActiveShiftKeys || isUpperCase());
    }

    // We're finished with this touch point, inform popup:
    if (mPopup) {
        mPopup->cancel();
    }

    longPressTimer.stop();

    if (q->debugTouchPoints) {
        q->logTouchPoint(tp, lookup.key, lookup.lastKey);
    }
    mTimestamp("MImAbstractKeyArea", "end");
}

QTouchEvent::TouchPoint
MImAbstractKeyAreaPrivate::createTouchPoint(int id,
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

MImAbstractKeyAreaPrivate::GravitationalLookupResult
MImAbstractKeyAreaPrivate::gravitationalKeyAt(const QPoint &pos,
                                              const QPoint &lastPos) const
{
    Q_Q(const MImAbstractKeyArea);

    // TODO: Needs explicit test coverage, maybe.
    MImAbstractKey *key = 0;
    MImAbstractKey *lastKey = 0;

    const qreal hGravity = q->style()->touchpointHorizontalGravity();
    const qreal vGravity = q->style()->touchpointVerticalGravity();

    key = q->keyAt(adjustedByGravity(MImAbstractKey::lastActiveKey(),
                                     pos, hGravity, vGravity).toPoint());
    lastKey = q->keyAt(adjustedByGravity(MImAbstractKey::lastActiveKey(),
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

bool MImAbstractKeyAreaPrivate::isInSpeedTypingMode(bool restartTimers)
{
    Q_Q(MImAbstractKeyArea);

    return ((restartTimers ? lastTouchPointPressEvent.restart()
                           : lastTouchPointPressEvent.elapsed()) < q->style()->idleVkbTimeout());
}

void MImAbstractKeyAreaPrivate::switchStyleMode()
{
    Q_Q(MImAbstractKeyArea);

    if (!q->style()->syncStyleModeWithKeyCount()) {
        return;
    }

    switch(section->keyCount()) {

    case 10:
        q->style().setModeKeys10();
        break;

    case 11:
        q->style().setModeKeys11();
        break;

    case 12:
        q->style().setModeKeys12();
        break;

    case 13:
        q->style().setModeKeys13();
        break;

    case 14:
        q->style().setModeKeys14();
        break;

    case 15:
        q->style().setModeKeys15();
        break;

    case 30:
        q->style().setModeKeys30();
        break;

    case 31:
        q->style().setModeKeys31();
        break;

    case 32:
        q->style().setModeKeys32();
        break;

    case 33:
        q->style().setModeKeys33();
        break;

    case 34:
        q->style().setModeKeys34();
        break;

    case 35:
        q->style().setModeKeys35();
        break;

    case 36:
        q->style().setModeKeys36();
        break;

    case 37:
        q->style().setModeKeys37();
        break;

    case 38:
        q->style().setModeKeys38();
        break;

    case 39:
        q->style().setModeKeys39();
        break;

    case 40:
        q->style().setModeKeys40();
        break;

    case 41:
        q->style().setModeKeys41();
        break;

    case 42:
        q->style().setModeKeys42();
        break;

    case 43:
        q->style().setModeKeys43();
        break;

    case 44:
        q->style().setModeKeys44();
        break;

    case 45:
        q->style().setModeKeys45();
        break;

    default:
        break;
    }
}

// actual class implementation
MImAbstractKeyArea::MImAbstractKeyArea(MImAbstractKeyAreaPrivate *privateData,
                                       bool usePopup,
                                       QGraphicsWidget *parent)
    : MStylableWidget(parent),
      mRelativeKeyBaseWidth(0),
      debugTouchPoints(style()->debugTouchPoints()),
      d_ptr(privateData)
{
    Q_D(MImAbstractKeyArea);

    d->mPopup = (usePopup ? PopupFactory::instance()->createPopup(this) : 0);

    // By default multi-touch is disabled
    if (d->enableMultiTouch) {
        setAcceptTouchEvents(true);
    }

    d->lastTouchPointPressEvent.restart();
    grabGesture(FlickGestureRecognizer::sharedGestureType());

    d->longPressTimer.setSingleShot(true);
    d->idleVkbTimer.setSingleShot(true);

    connect(&d->longPressTimer, SIGNAL(timeout()),
            this, SLOT(handleLongKeyPressed()));

    connect(&d->idleVkbTimer, SIGNAL(timeout()),
            this, SLOT(handleIdleVkb()));

    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()),
            Qt::UniqueConnection);

    d->switchStyleMode();
}

MImAbstractKeyArea::~MImAbstractKeyArea()
{
    delete d_ptr;
}

void MImAbstractKeyArea::setInputMethodMode(M::InputMethodMode inputMethodMode)
{
    MImAbstractKeyAreaPrivate::InputMethodMode = inputMethodMode;
}

qreal MImAbstractKeyArea::relativeKeyBaseWidth() const
{
    return mRelativeKeyBaseWidth;
}

const LayoutData::SharedLayoutSection &MImAbstractKeyArea::sectionModel() const
{
    Q_D(const MImAbstractKeyArea);

    return d->section;
}

void MImAbstractKeyArea::updatePopup(MImAbstractKey *key)
{
    Q_D(MImAbstractKeyArea);

    if (!d->mPopup) {
        return;
    }

    if (!key) {
        d->mPopup->cancel();
        return;
    }

    const QRectF &buttonRect = key->buttonRect();
    // IM FW guarantees that a scene position matches with
    // screen position, so we can use mapToScene to calculate screen position:
    const QPoint pos = mapToScene(buttonRect.topLeft()).toPoint();

    MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&finder);

    d->mPopup->updatePos(buttonRect.topLeft(), pos, buttonRect.toRect().size());
    d->mPopup->handleKeyPressedOnMainArea(key,
                                          (finder.deadKey() ? finder.deadKey()->label() : QString()),
                                          d->isUpperCase());
}

int MImAbstractKeyArea::maxColumns() const
{
    Q_D(const MImAbstractKeyArea);

    return d->section->maxColumns();
}

int MImAbstractKeyArea::rowCount() const
{
    Q_D(const MImAbstractKeyArea);

    return d->rowCount();
}

void
MImAbstractKeyArea::handleVisibilityChanged(bool visible)
{
    Q_D(MImAbstractKeyArea);

    if (!visible) {
        if (d->mPopup) {
            d->mPopup->setVisible(false);
        }

        MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::FindDeadKey);
        MImAbstractKey::visitActiveKeys(&finder);

        unlockDeadKeys(finder.deadKey());
    }
}

void
MImAbstractKeyArea::switchLevel(int level)
{
    Q_D(MImAbstractKeyArea);

    if (level != d->currentLevel) {
        d->currentLevel = level;

        // Update uppercase / lowercase
        MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::FindDeadKey);
        MImAbstractKey::visitActiveKeys(&finder);

        updateKeyModifiers(finder.deadKey() ? finder.deadKey()->label().at(0) : '\0');

        update();
    }
}

int MImAbstractKeyArea::level() const
{
    Q_D(const MImAbstractKeyArea);

    return d->currentLevel;
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
    Q_D(MImAbstractKeyArea);

    if (d->enableMultiTouch) {
        return;
    }

    d->touchPointPressed(d->createTouchPoint(0, Qt::TouchPointPressed,
                                             mapToScene(ev->pos()),
                                             mapToScene(gLastMousePos)));

    gLastMousePos = ev->pos();
}

void MImAbstractKeyArea::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    Q_D(MImAbstractKeyArea);

    if (d->enableMultiTouch) {
        return;
    }

    d->touchPointMoved(d->createTouchPoint(0, Qt::TouchPointMoved,
                                           mapToScene(ev->pos()),
                                           mapToScene(gLastMousePos)));

    gLastMousePos = ev->pos();
}

void MImAbstractKeyArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    Q_D(MImAbstractKeyArea);

    if (scene()->mouseGrabberItem() == this) {
        // Ungrab mouse explicitly since we probably used grabMouse() to get it.
        ungrabMouse();
    }

    if (d->enableMultiTouch) {
        return;
    }

    d->touchPointReleased(d->createTouchPoint(0, Qt::TouchPointReleased,
                                              mapToScene(ev->pos()),
                                              mapToScene(gLastMousePos)));

    gLastMousePos = ev->pos();
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
    Q_D(MImAbstractKeyArea);

    // If mimabstractkeyarea is hidden without mouseReleaseEvent
    // the enabled <flicked> would stay true if mouse
    // grab is obtained again without mousePressEvent.
    // This would ignore mouseReleaseEvent and would not cause keyClicked.
    d->wasGestureTriggered = false;

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
    Q_D(MImAbstractKeyArea);

    // Make sure popup can respond to mouse grab removal:
    if (d->mPopup) {
        d->mPopup->cancel();
    }

    if (d->enableMultiTouch && MImAbstractKey::lastActiveKey()) {
        MImAbstractKey::lastActiveKey()->resetTouchPointCount();
    }

    d->longPressTimer.stop();
}

bool MImAbstractKeyArea::event(QEvent *ev)
{
    Q_D(MImAbstractKeyArea);

    bool eaten = false;
    QString start, end;
    start = QString("%1|start").arg(ev->type());
    end = QString("%1|end").arg(ev->type());
    mTimestamp("MImAbstractKeyArea", start);

    if (ev->type() == QEvent::Gesture) {
        const Qt::GestureType flickGestureType = FlickGestureRecognizer::sharedGestureType();
        FlickGesture *flickGesture = static_cast<FlickGesture *>(static_cast<QGestureEvent *>(ev)->gesture(flickGestureType));

        if (flickGesture) {
            d->handleFlickGesture(flickGesture);
            eaten = true;
        }
    } else if (ev->type() == QEvent::TouchBegin
               || ev->type() == QEvent::TouchUpdate
               || ev->type() == QEvent::TouchEnd) {
        QTouchEvent *touch = static_cast<QTouchEvent*>(ev);

        foreach (const QTouchEvent::TouchPoint &tp, touch->touchPoints()) {

            switch (tp.state()) {
            case Qt::TouchPointPressed:
                d->touchPointPressed(tp);
                break;

            case Qt::TouchPointMoved:
                d->touchPointMoved(tp);
                break;

            case Qt::TouchPointReleased:
                d->touchPointReleased(tp);
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
    Q_D(MImAbstractKeyArea);

    if (!d->mPopup) {
        return;
    }

    d->mPopup->setVisible(false);
}

void MImAbstractKeyArea::drawReactiveAreas(MReactionMap *,
                                           QGraphicsView *)
{
    // Empty default implementation. Geometries of buttons are known by derived classes.
}

const PopupBase &MImAbstractKeyArea::popup() const
{
    Q_D(const MImAbstractKeyArea);

    return *d->mPopup;
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
    Q_D(MImAbstractKeyArea);

    // We currently don't allow active dead key level changing. If we did,
    // we should update activeDeadKey level before delivering its accent to
    // other keys.
    bool shift = (d->currentLevel == 1);
    modifiersChanged(shift, accent);
}

void MImAbstractKeyArea::modifiersChanged(bool,
                                          const QChar &)
{
    // Empty default implementation
}

void MImAbstractKeyArea::enableHorizontalFlick(bool enable)
{
    Q_D(MImAbstractKeyArea);

    d->allowedHorizontalFlick = enable;
}

void MImAbstractKeyArea::onThemeChangeCompleted()
{
    Q_D(MImAbstractKeyArea);

    // TODO: update all other CSS attributes that are mapped to members.
    d->switchStyleMode();
    updateKeyGeometries(size().width());
}

const MImAbstractKeyAreaStyleContainer &MImAbstractKeyArea::baseStyle() const
{
    return style();
}

void MImAbstractKeyArea::handleLongKeyPressed()
{
    Q_D(MImAbstractKeyArea);
    MImAbstractKey *const lastActiveKey = MImAbstractKey::lastActiveKey();

    if (!lastActiveKey) {
        return;
    }

    MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&finder);

    const QString accent = (finder.deadKey() ? finder.deadKey()->label()
                                           : QString());

    if (d->mPopup) {
        d->mPopup->handleLongKeyPressedOnMainArea(lastActiveKey, accent, d->isUpperCase());
    }

    emit longKeyPressed(lastActiveKey, accent, d->isUpperCase());
}

void MImAbstractKeyArea::handleIdleVkb()
{
    grabGesture(FlickGestureRecognizer::sharedGestureType());
}

void MImAbstractKeyArea::reset(bool resetCapsLock)
{
    Q_D(MImAbstractKeyArea);

    if (scene()->mouseGrabberItem() == this) {
        // Ungrab mouse explicitly since we probably used grabMouse() to get it.
        ungrabMouse();
    }

    if (d->mPopup) {
        d->mPopup->cancel();
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
    const qreal offset = baseStyle()->touchpointVerticalOffset();

    if (rect.top() >= offset) {
        rect.setTop(rect.top() + offset);
    }
    const qreal newBottom(rect.bottom() + offset);
    if (newBottom <= size().height()) {
        rect.setBottom(newBottom);
    }

    return rect;
}
