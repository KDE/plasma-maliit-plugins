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
#include "keybuttonarea.h"
#include "limitedtimer.h"
#include "popupbase.h"
#include "popupfactory.h"

#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <QDebug>
#include <QEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QHash>
#include <QKeyEvent>
#include <mtimestamp.h>

namespace
{
    const int FlickTime     = 150;
    const int LongPressTime = 1000;

    const int FlickDownThreshold = 50;
    const int FlickUpThreshold = 50;
    const int FlickLeftRightVerticalThreshold = 50;
    const int FlickLeftRightHorizontalThreshold = 50;

    // Minimal distinguishable cursor/finger movement
    const qreal MovementThreshold = 5.0;

    const qreal ZValueButtons = 0.0;
};

KeyButtonArea::KeyButtonArea(MVirtualKeyboardStyleContainer *style,
                             QSharedPointer<const LayoutSection> sectionModel,
                             ButtonSizeScheme buttonSizeScheme,
                             bool usePopup,
                             QGraphicsWidget *parent)
    : MWidget(parent),
      currentLevel(0),
      popup(PopupFactory::instance()->createPopup(*style, this)),
      styleContainer(style),
      flickTimer(new LimitedTimer(this)),
      newestTouchPointId(-1),
      longPressTimer(new LimitedTimer(this)),
      accurateMode(false),
      flicked(false),
      enableMultiTouch(false),
      activeDeadkey(0),
      feedbackPlayer(0),
      section(sectionModel),
      buttonSizeScheme(buttonSizeScheme),
      usePopup(usePopup)
{
    // By default multi-touch is enabled.
    setMultiTouch(true);

    flickTimer->setSingleShot(true);
    flickTimer->setInterval(FlickTime);
    connect(flickTimer, SIGNAL(timeout()), this, SLOT(flickCheck()));

    longPressTimer->setSingleShot(true);
    longPressTimer->setInterval(LongPressTime);
    connect(longPressTimer, SIGNAL(timeout()), this, SLOT(accurateStart()));
    connect(longPressTimer, SIGNAL(timeout()), this, SLOT(popupStart()));

    popup->hidePopup();

    feedbackPlayer = MComponentData::feedbackPlayer();
}

KeyButtonArea::~KeyButtonArea()
{
    delete flickTimer;
    delete longPressTimer;
    delete popup;
}

void KeyButtonArea::buttonInformation(int row, int column, const VKBDataKey *&dataKey, QSize &size, bool &stretchesHorizontally)
{
    // We share the same button instance with different key bindings.
    dataKey = section->getVKBKey(row, column);

    if (dataKey) {
        // Preferred button size and stretch is based on level=0 binding. It's safe because
        // buttons are the same size in same locations.
        const KeyBinding::KeyAction action = dataKey->binding(false)->action();

        size = buttonSizeByAction(action);

        // Make space button stretch.
        if (action == KeyBinding::ActionSpace) {
            stretchesHorizontally = true;
        } else {
            stretchesHorizontally = false;
        }
    } else {
        size = QSize();
        stretchesHorizontally = false;
    }
}

QSharedPointer<const LayoutSection> KeyButtonArea::sectionModel() const
{
    return section;
}

ISymIndicator *KeyButtonArea::symIndicator()
{
	return 0;
}

void KeyButtonArea::updatePopup(const QPoint &pointerPosition, const IKeyButton *key)
{
    // Use prefetched key if given.
    if (!key) {
        key = keyAt(pointerPosition);
    }

    if (!key) {
        return;
    }

    const QRect &buttonRect = key->buttonRect();
    // mimframework guarantees that scene positions matches with
    // screen position, so we can use mapToScene to calculate screen position
    const QPoint pos = mapToScene(buttonRect.topLeft()).toPoint();

    popup->updatePos(buttonRect.topLeft(), pos, buttonRect.size());

    // Get direction for finger position from key center and normalize components.
    QPointF direction(pointerPosition - buttonRect.center());
    direction.rx() /= buttonRect.width();
    direction.ry() /= buttonRect.height();
    popup->setFingerPos(direction);
    popup->setTargetButton(key);

    popup->showPopup();
}

int KeyButtonArea::maxColumns() const
{
    return section->maxColumns();
}

int KeyButtonArea::rowCount() const
{
    return section->rowCount();
}

void
KeyButtonArea::onHide()
{
    clearActiveKeys();
    unlockDeadkeys();
}

void
KeyButtonArea::switchLevel(int level)
{
    if (level != currentLevel) {
        currentLevel = level;

        // Update uppercase / lowercase
        updateButtonModifiers();

        update();
    }
}

int KeyButtonArea::level() const
{
    return currentLevel;
}

void KeyButtonArea::setShiftStatus(bool /*shiftOn*/, bool /*capslock*/)
{
    // Empty default implementation
}

bool
KeyButtonArea::isObservableMove(const QPointF &prevPos, const QPointF &pos)
{
    qreal movement = qAbs(prevPos.x() - pos.x()) + qAbs(prevPos.y() - pos.y());

    return movement >= MovementThreshold;
}

KeyEvent KeyButtonArea::keyToKeyEvent(const IKeyButton &key, QKeyEvent::Type eventType) const
{
    const bool shift = (currentLevel == 1);
    KeyEvent event;

    if (activeDeadkey != NULL) {
        event = key.key().toKeyEvent(eventType, activeDeadkey->label().at(0), shift);
    } else {
        event = key.key().toKeyEvent(eventType, shift);
    }

    return event;
}

void KeyButtonArea::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    const int newWidth = static_cast<int>(event->newSize().width());
    if (newWidth != static_cast<int>(event->oldSize().width())) {
        // All main keyboards should take the whole available width so we restrict
        // the button width here. This is applied only if we have equal button width.
        int equalButtonWidth = -1; // the new width if it's same for all
        if ((buttonSizeScheme == ButtonSizeEqualExpanding)
             || (buttonSizeScheme == ButtonSizeEqualExpandingPhoneNumber)) {
            const int HorizontalSpacing = style()->spacingHorizontal();
            const int MaxButtonWidth = qRound(static_cast<qreal>(newWidth + HorizontalSpacing)
                                              / static_cast<qreal>(section->maxColumns())
                                              - HorizontalSpacing);

            equalButtonWidth = MaxButtonWidth;
        }
        updateButtonGeometries(newWidth, equalButtonWidth);
    }
}

void
KeyButtonArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!enableMultiTouch) {
        // Qt always assigns zero to the first touch point, so pass id = 0.
        touchPointPressed(event->pos().toPoint(), 0);
    }
}


void
KeyButtonArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (scene()->mouseGrabberItem() == this) {
        // Ungrab mouse explicitly since we probably used grabMouse() to get it.
        ungrabMouse();
    }

    if (!enableMultiTouch) {
        touchPointReleased(event->pos().toPoint(), 0);
    }
}


void KeyButtonArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!enableMultiTouch) {
        touchPointMoved(event->pos().toPoint(), 0);
    }
}

void KeyButtonArea::setActiveKey(IKeyButton *key, TouchPointInfo &tpi)
{
    // Selected buttons are currently skipped.

    if (tpi.activeKey && (tpi.activeKey != key)) {
        // Release key
        tpi.activeKey->setDownState(false);
        KeyEvent releaseEvent = keyToKeyEvent(*tpi.activeKey, QEvent::KeyRelease);
        emit keyReleased(releaseEvent);
        tpi.activeKey = 0;
    }

    if (key && (tpi.activeKey != key)) {
        // Press key
        tpi.activeKey = key;
        tpi.activeKey->setDownState(true);
        KeyEvent pressEvent = keyToKeyEvent(*tpi.activeKey, QEvent::KeyPress);
        emit keyPressed(pressEvent);
    }
}

void KeyButtonArea::clearActiveKeys()
{
    for (int i = 0; i < touchPoints.count(); ++i) {
        setActiveKey(0, touchPoints[i]);
    }
}

void KeyButtonArea::click(const IKeyButton *key)
{
    if (!key->isDeadKey()) {
        KeyEvent clickEvent = keyToKeyEvent(*key, QEvent::KeyRelease);

        unlockDeadkeys();

        // Check if we need to disable accurate mode
        accurateCheckContent(key->label());

        emit keyClicked(clickEvent);
    } else {
        clickAtDeadkey(key);
        update();
    }
}

QVariant KeyButtonArea::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemVisibleChange && !value.toBool()) {
        onHide();
    }
    return QGraphicsItem::itemChange(change, value);
}

void KeyButtonArea::grabMouseEvent(QEvent */*event*/)
{
    // If keybuttonarea is hidden without mouseReleaseEvent
    // the enabled <flicked> would stay true if mouse
    // grab is obtained again without mousePressEvent.
    // This would ignore mouseReleaseEvent and would not cause keyClicked.
    flicked = false;
}

void KeyButtonArea::ungrabMouseEvent(QEvent */*event*/)
{
    // longPressTimer is running if flicked without releasing
    // (or releasing after this keybuttonarea is hidden)
    longPressTimer->stop();

    // Make sure popup is hidden even if mouse grab
    // is lost without mouse release event.
    popup->hidePopup();
}

bool KeyButtonArea::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin
        || event->type() == QEvent::TouchUpdate
        || event->type() == QEvent::TouchEnd) {
        QTouchEvent *touch = static_cast<QTouchEvent*>(event);

        if (event->type() == QEvent::TouchBegin) {
            touchPoints.clear();
        }

        foreach (const QTouchEvent::TouchPoint &tp, touch->touchPoints()) {

            switch (tp.state()) {
            case Qt::TouchPointPressed:
                touchPointPressed(tp.pos().toPoint(), tp.id());
                break;
            case Qt::TouchPointMoved:
                touchPointMoved(tp.pos().toPoint(), tp.id());
                break;
            case Qt::TouchPointReleased:
                touchPointReleased(tp.pos().toPoint(), tp.id());
                break;
            default:
                break;
            }
        }

        return true;
    }

    return MWidget::sceneEvent(event);
}

void KeyButtonArea::touchPointPressed(const QPoint &pos, int id)
{
    newestTouchPointId = id;

    // Create new TouchPointInfo structure and overwrite any previous one.
    touchPoints[id] = TouchPointInfo();
    TouchPointInfo &tpi = touchPoints[id];
    tpi.pos = pos;
    tpi.initialPos = pos;

    // Reset flick check.
    flicked = false;
    flickTimer->start();

    // Reset long-press check.
    longPressTimer->start();

    IKeyButton *key = keyAt(pos);
    if (!key) {
        return;
    }

    mTimestamp("KeyButtonArea", key->label());

    tpi.initialKey = key;
    tpi.fingerInsideArea = true;

    if (accurateMode) {
        // Stop accurate mode if pressed key makes accurate mode irrelevant
        accurateCheckContent(key->label());

        // Check if accurate mode is still on.
        // Show popup in accurate mode.
        if (accurateMode && usePopup) {
            updatePopup(pos, key);
        }
    }

    setActiveKey(key, tpi);
}

void KeyButtonArea::touchPointMoved(const QPoint &pos, int id)
{
    TouchPointInfo &tpi = touchPoints[id];

    if (!isObservableMove(tpi.pos, pos))
        return;

    tpi.pos = pos;

    // Check if finger is on a key.
    IKeyButton *key = keyAt(pos);
    if (key) {
        tpi.fingerInsideArea = true;

        if ((tpi.activeKey != key) && accurateMode && feedbackPlayer) {
            // Finger has slid from a key to an adjacent one.
            feedbackPlayer->play(MFeedbackPlayer::Press);
        }

        // If popup is visible, always update the position,
        // even if accurate mode is not enabled. This is for
        // Sym view, who doesn't care about accurate mode.
        if (usePopup && (accurateMode || popup->isPopupVisible())) {
            updatePopup(pos, key);
        }

        if (tpi.activeKey != key) {
            // Use has to keep finger still to generate long press.
            // isObservableMove() makes this task easier for the user.
            if (longPressTimer->isActive())
                longPressTimer->start();
        }
    } else {
        if (tpi.fingerInsideArea && accurateMode && feedbackPlayer) {
            feedbackPlayer->play(MFeedbackPlayer::Cancel);
        }
        // Finger has slid off the keys
        if (tpi.fingerInsideArea) {
            popup->hidePopup();
        }

        longPressTimer->stop();
        tpi.fingerInsideArea = false;
    }

    setActiveKey(key, tpi);
}

void KeyButtonArea::touchPointReleased(const QPoint &pos, int id)
{
    TouchPointInfo &tpi = touchPoints[id];
    tpi.pos = pos;
    tpi.fingerInsideArea = false;

    // No more long-press triggerings, although would still be possible to make
    // with another touch point.
    longPressTimer->stop();

    // Same thing here, hide popup although otherwise could be shown on
    // another touch point.
    popup->hidePopup();

    // Stop flick timer but save current state first.
    const bool flickTimerWasActive = flickTimer->isActive();
    flickTimer->stop();

    IKeyButton *key = keyAt(pos);

    if (key) {
        mTimestamp("KeyButtonArea", key->label());

        // It's presumably possible that we're getting this release event on top
        // of another after press event (of another key) without first getting a
        // move event (or at least such move event that we handle).  Which means
        // that we must send release event for the previous key and press event
        // for this key before sending release and clicked events for this key.
        setActiveKey(key, tpi); // in most cases, does nothing
        setActiveKey(0, tpi); // release key

        // Check if keyboard was flicked or
        // release happens fast and far enough from the start.
        // If so, then skip click.
        // Altough we associate flick gesture with the newest touch point
        // it can still mistrigger because sometimes, when clicked nearly
        // simultaneously, we receive a single move between two pressed points.
        if ((id != newestTouchPointId)
            || !(flicked || (flickTimerWasActive && flickCheck()))) {
            click(key);
        }
    } else {
        setActiveKey(0, tpi);
    }
}

void
KeyButtonArea::clickAtDeadkey(const IKeyButton *deadKey)
{
    Q_ASSERT(deadKey);

    if (deadKey == activeDeadkey) {
        unlockDeadkeys();
    } else {
        activeDeadkey = deadKey;

        updateButtonModifiers();
    }
}

void
KeyButtonArea::unlockDeadkeys()
{
    if (activeDeadkey) {
        activeDeadkey = 0;
        updateButtonModifiers();
    }
}

void KeyButtonArea::setMultiTouch(bool enable)
{
    enableMultiTouch = enable;
    setAcceptTouchEvents(enable);
}

void KeyButtonArea::popupStart()
{
    if (usePopup && touchPoints.contains(newestTouchPointId)) {
        updatePopup(touchPoints[newestTouchPointId].pos);
    }
}

void
KeyButtonArea::accurateStart()
{
    if (!accurateMode) {
        accurateMode = true;
        emit accurateModeStarted();
    }
}

void
KeyButtonArea::accurateStop()
{
    if (accurateMode) {
        accurateMode = false;
        emit accurateModeStopped();
    }
}

void
KeyButtonArea::accurateCheckContent(const QString &content)
{
    if (!content.isEmpty()) {
        QChar c = content.at(0);
        bool unicodeLetter =
            c.category() >= QChar::Letter_Uppercase &&
            c.category() <= QChar::Letter_Other;
        if (!unicodeLetter) {
            accurateStop();
        }
    }
}

bool
KeyButtonArea::isAccurateMode() const
{
    return accurateMode;
}

bool
KeyButtonArea::isPopupActive() const
{
    return popup->isPopupVisible();
}

void KeyButtonArea::drawReactiveAreas(MReactionMap */*reactionMap*/, QGraphicsView */*view*/)
{
    // Empty default implementation. Geometries of buttons are known by derived classes.
}

bool
KeyButtonArea::flickCheck()
{
    if (flicked || !touchPoints.contains(newestTouchPointId))
        return false;

    TouchPointInfo &flickedPoint = touchPoints[newestTouchPointId];

    bool res = false;
    QPoint diff = flickedPoint.initialPos - flickedPoint.pos;

    if (qAbs(diff.y()) < FlickLeftRightVerticalThreshold) {
        if (diff.x() > FlickLeftRightHorizontalThreshold) {
            res = true;
            emit flickLeft();
        } else if (diff.x() < -FlickLeftRightHorizontalThreshold) {
            res = true;
            emit flickRight();
        }
    } else if (qAbs(diff.x()) < FlickDownThreshold) {
        if (diff.y() < - FlickDownThreshold) {
            res = true;
            emit flickDown();
        } else if (diff.y() > FlickUpThreshold) {
            const KeyBinding *binding = 0;
            res = true;
            if (flickedPoint.initialKey) {
                binding = &flickedPoint.initialKey->binding();
            }
            emit flickUp(binding);
        }
    }

    if (res) {
        popup->hidePopup();
        flicked = true;
    }
    return res;
}

const MVirtualKeyboardStyleContainer &KeyButtonArea::style() const
{
    return *styleContainer;
}

const PopupBase &KeyButtonArea::popupWidget() const
{
    return *popup;
}

QSize KeyButtonArea::buttonSizeByAction(KeyBinding::KeyAction action) const
{
    QSize buttonSize;

    if (buttonSizeScheme == ButtonSizeEqualExpanding) {
        buttonSize = style()->keyNormalSize();
    } else if (buttonSizeScheme == ButtonSizeEqualExpandingPhoneNumber) {
        buttonSize = style()->keyPhoneNumberNormalSize();
    } else if (buttonSizeScheme == ButtonSizeFunctionRow) {
        switch (action) {
        case KeyBinding::ActionSpace:
        case KeyBinding::ActionReturn:
            // Will be stretched horizontally
            buttonSize = style()->keyFunctionNormalSize();
            break;
        case KeyBinding::ActionBackspace:
        case KeyBinding::ActionShift:
            buttonSize = style()->keyFunctionLargeSize();
            break;
        default:
            buttonSize = style()->keyFunctionNormalSize();
        }
    } else if (buttonSizeScheme == ButtonSizeFunctionRowNumber) {
        if (action == KeyBinding::ActionBackspace) {
            buttonSize = style()->keyNumberBackspaceSize();
        } else {
            buttonSize = style()->keyNormalSize();
        }
    } else if (buttonSizeScheme == ButtonSizeSymbolView) {
        buttonSize = style()->keySymNormalSize();
    }

    return buttonSize;
}

void KeyButtonArea::updateButtonModifiers()
{
    bool shift = (currentLevel == 1);
    const QChar accent(activeDeadkey ? activeDeadkey->label().at(0) : '\0');

    modifiersChanged(shift, accent);
}

void KeyButtonArea::modifiersChanged(bool /*shift*/, const QChar /*accent*/)
{
    // Empty default implementation
}

KeyButtonArea::TouchPointInfo::TouchPointInfo()
    : fingerInsideArea(false),
      activeKey(0),
      initialKey(0),
      initialPos(),
      pos()
{
}
