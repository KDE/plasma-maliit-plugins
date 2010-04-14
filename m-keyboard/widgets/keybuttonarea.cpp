/* * This file is part of m-keyboard *
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
      shiftButton(0),
      currentLevel(0),
      popup(PopupFactory::instance()->createPopup(*style, this)),
      styleContainer(style),
      flickTimer(new LimitedTimer(this)),
      longPressTimer(new LimitedTimer(this)),
      accurateMode(false),
      flicked(false),
      activeDeadkey(0),
      activeKey(0),
      flickedKey(0),
      feedbackPlayer(0),
      section(sectionModel),
      buttonSizeScheme(buttonSizeScheme),
      usePopup(usePopup)
{
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
    unlockDeadkeys();
}

void
KeyButtonArea::switchLevel(int level, bool capslock)
{
    currentLevel = level;

    if (shiftButton) {
        shiftButton->setSelected(capslock);
    }

    // Update uppercase / lowercase
    updateButtonModifiers();

    update();
}

bool
KeyButtonArea::isObservableMove(const QPointF &prevPos, const QPointF &pos)
{
    qreal movement = qAbs(prevPos.x() - pos.x()) + qAbs(prevPos.y() - pos.y());

    return movement >= MovementThreshold;
}

KeyEvent KeyButtonArea::keyToKeyEvent(const IKeyButton &key, QKeyEvent::Type eventType) const
{
    Qt::KeyboardModifiers modifiers = (currentLevel == 1) ? Qt::ShiftModifier : Qt::NoModifier;
    KeyEvent event;

    if (activeDeadkey != NULL) {
        event = key.key().toKeyEvent(eventType, activeDeadkey->label().at(0), modifiers);
    } else {
        event = key.key().toKeyEvent(eventType, modifiers);
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
    pointerPos = event->pos().toPoint();

    IKeyButton *key = keyAt(pointerPos);

    if (!key) {
        event->ignore();
        return;
    }

    mTimestamp("KeyButtonArea", key->label());

    fingerInsideArea = true;

    longPressTimer->start();

    // Stop accurate mode if pressed key makes accurate mode irrelevant
    if (accurateMode)
        accurateCheckContent(key->label());

    // Check if accurate mode is still on.
    if (accurateMode) {
        // Show popup in accurate mode.
        if (usePopup) {
            updatePopup(pointerPos, key);
        }
    }

    if (flickTimer->isActive()) {
        flickTimer->stop();
    }

    flicked = false;
    flickStartPos = pointerPos;
    flickedKey = key;
    flickTimer->start();

    setActiveKey(key);
}


void
KeyButtonArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    bool flickTimerActive = flickTimer->isActive();
    longPressTimer->stop();
    flickTimer->stop();
    fingerInsideArea = false;
    popup->hidePopup();

    pointerPos = event->pos().toPoint();

    IKeyButton *key = keyAt(pointerPos);

    if (key) {
        mTimestamp("KeyButtonArea", key->label());
    }

    // It's presumably possible that we're getting this release event on top
    // of another after press event (of another key) without first getting a
    // move event (or at least such move event that we handle).  Which means
    // that we must send release event for the previous key and press event
    // for this key before sending release and clicked events for this key.
    if (key && (key != activeKey)) {
        setActiveKey(key);
    }

    if (scene()->mouseGrabberItem() == this) {
        // Ungrab mouse explicitly since we probably used grabMouse() to get it.
        // Ungrab event handler will clear active key.
        ungrabMouse();
    } else {
        setActiveKey(0);
    }

    // Check if keyboard was flicked or
    // release happens fast and far enough from the start
    if (flicked || (flickTimerActive && flickCheck() == true)) {
        return;
    }

    // Handle click
    if (key) {
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
}


void KeyButtonArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isObservableMove(pointerPos, event->pos()))
        return;

    pointerPos = event->pos().toPoint();

    // Check if finger is on a key.
    IKeyButton *key = keyAt(pointerPos);
    if (!key) {

        // Finger has slid off the keys
        if (fingerInsideArea) {
            if (accurateMode && feedbackPlayer)
                feedbackPlayer->play(MFeedbackPlayer::Cancel);

            popup->hidePopup();

            // Don't show button as pressed, and send release event.
            setActiveKey(0);
        }

        fingerInsideArea = false;
        longPressTimer->stop();
    } else {
        fingerInsideArea = true;

        // If popup is visible, always update the position,
        // even if accurate mode is not enabled. This is for
        // Sym view, who doesn't care about accurate mode.
        if (usePopup && (accurateMode || popup->isPopupVisible())) {
            updatePopup(pointerPos, key);
        }

        if (activeKey != key) {
            setActiveKey(key);

            if (accurateMode && feedbackPlayer) {
                // Finger has slid from a key to an adjacent one.
                feedbackPlayer->play(MFeedbackPlayer::Press);
            }

            // Use has to keep finger still to generate long press.
            // isObservableMove() makes this task easier for the user.
            if (longPressTimer->isActive())
                longPressTimer->start();
        }
    } // inside area
}

void KeyButtonArea::setActiveKey(IKeyButton *key)
{
    // Selected buttons are currently skipped.

    if (activeKey && (activeKey != key)) {
        // Release key
        activeKey->setDownState(false);
        KeyEvent releaseEvent = keyToKeyEvent(*activeKey, QEvent::KeyRelease);
        emit keyReleased(releaseEvent);
        activeKey = 0;
    }

    if (key && (activeKey != key)) {
        // Press key
        activeKey = key;
        activeKey->setDownState(true);
        KeyEvent pressEvent = keyToKeyEvent(*activeKey, QEvent::KeyPress);
        emit keyPressed(pressEvent);
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

    // Release any key we have currently active
    setActiveKey(0);
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

void KeyButtonArea::popupStart()
{
    if (usePopup) {
        updatePopup(pointerPos);
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

void KeyButtonArea::drawReactiveAreas(DuiReactionMap */*reactionMap*/, QGraphicsView */*view*/)
{
    // Empty default implementation. Geometries of buttons are known by derived classes.
}

bool
KeyButtonArea::flickCheck()
{
    if (flicked)
        return false;

    bool res = false;
    QPointF diff = flickStartPos - pointerPos;

    if (diff.y() > -FlickLeftRightVerticalThreshold &&
            diff.y() < FlickLeftRightVerticalThreshold) {
        if (diff.x() > FlickLeftRightHorizontalThreshold) {
            res = true;
            emit flickLeft();
        } else if (diff.x() < -FlickLeftRightHorizontalThreshold) {
            res = true;
            emit flickRight();
        }
    } else if (diff.x() > - FlickDownThreshold && diff.x() < FlickDownThreshold) {
        if (diff.y() < - FlickDownThreshold) {
            res = true;
            emit flickDown();
        } else if (diff.y() > FlickUpThreshold) {
            const KeyBinding *binding = 0;
            res = true;
            if (flickedKey) {
                binding = &flickedKey->binding();
                flickedKey = 0;
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
