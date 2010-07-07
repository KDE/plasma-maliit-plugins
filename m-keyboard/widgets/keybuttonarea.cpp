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
#include "keybuttonarea.h"
#include "limitedtimer.h"
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
    const int GestureTimeOut = 1000;
    const int LongPressTime  = 0;
    const qreal ZValueButtons = 0.0;

    // Minimal distinguishable cursor/finger movement
    const qreal MovementThreshold = 5.0;

    // For gesture thresholds: How many pixels translate to one counted move event.
    const qreal PixelsToMoveEventsFactor = 0.02;

    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
}

M::InputMethodMode KeyButtonArea::InputMethodMode;

KeyButtonArea::KeyButtonArea(const MVirtualKeyboardStyleContainer *style,
                             QSharedPointer<const LayoutSection> sectionModel,
                             ButtonSizeScheme buttonSizeScheme,
                             bool usePopup,
                             QGraphicsWidget *parent)
    : MWidget(parent),
      currentLevel(0),
      popup(PopupFactory::instance()->createPopup(*style, this)),
      styleContainer(style),
      newestTouchPointId(-1),
      longPressTimer(new LimitedTimer(this)),
      accurateMode(false),
      wasGestureTriggered(false),
      enableMultiTouch(MGConfItem(MultitouchSettings).value().toBool()),
      activeDeadkey(0),
      feedbackPlayer(0),
      section(sectionModel),
      buttonSizeScheme(buttonSizeScheme),
      usePopup(usePopup)
{
    // By default multi-touch is disabled
    if (enableMultiTouch) {
        setAcceptTouchEvents(true);
    }

    grabGesture(FlickGestureRecognizer::sharedGestureType());

    longPressTimer->setSingleShot(true);
    longPressTimer->setInterval(LongPressTime);
    connect(longPressTimer, SIGNAL(timeout()), this, SLOT(accurateStart()));
    connect(longPressTimer, SIGNAL(timeout()), this, SLOT(popupStart()));

    popup->hidePopup();

    feedbackPlayer = MComponentData::feedbackPlayer();

    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()),
            Qt::UniqueConnection);
}

KeyButtonArea::~KeyButtonArea()
{
    delete longPressTimer;
    delete popup;
}

void KeyButtonArea::setInputMethodMode(M::InputMethodMode inputMethodMode)
{
    InputMethodMode = inputMethodMode;
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

void KeyButtonArea::setShiftState(ModifierState /*newShiftState*/)
{
    // Empty default implementation
}

bool
KeyButtonArea::isObservableMove(const QPointF &prevPos, const QPointF &pos)
{
    qreal movement = qAbs(prevPos.x() - pos.x()) + qAbs(prevPos.y() - pos.y());

    return movement >= MovementThreshold;
}

void KeyButtonArea::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    const int newWidth = static_cast<int>(event->newSize().width());

    if (newWidth != static_cast<int>(event->oldSize().width())) {
        updateButtonGeometriesForWidth(newWidth);
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
    QString accent;

    if (activeDeadkey) {
        accent = activeDeadkey->label();
    }

    if (tpi.activeKey && (tpi.activeKey != key)) {
        // Release key
        tpi.activeKey->setDownState(false);
        // odd level numbers are upper case,
        // even level numbers are lower case
        emit keyReleased(tpi.activeKey, accent, level() % 2);
        tpi.activeKey = 0;
    }

    if (key && (tpi.activeKey != key)) {
        // Press key
        tpi.activeKey = key;
        tpi.activeKey->setDownState(true);
        emit keyPressed(tpi.activeKey, accent, level() % 2);
    }
}

void KeyButtonArea::clearActiveKeys()
{
    for (int i = 0; i < touchPoints.count(); ++i) {
        setActiveKey(0, touchPoints[i]);
    }
}

void KeyButtonArea::click(IKeyButton *key)
{
    if (!key->isDeadKey()) {
        QString accent;

        if (activeDeadkey) {
            accent = activeDeadkey->label();
        }

        // Check if we need to disable accurate mode
        accurateCheckContent(key->label());

        unlockDeadkeys();

        emit keyClicked(key, accent, level() % 2);
    } else if (key == activeDeadkey) {
        unlockDeadkeys();
    } else {
        // Deselect previous dead key, if any
        if (activeDeadkey) {
            activeDeadkey->setSelected(false);
        }

        activeDeadkey = key;
        activeDeadkey->setSelected(true);

        updateButtonModifiers();
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
    wasGestureTriggered = false;

    const qreal ScalingFactor = style()->flickGestureThresholdRatio();
    const int HorizontalThreshold = static_cast<int>(boundingRect().width() * ScalingFactor);
    const int VerticalThreshold = static_cast<int>(boundingRect().height() * ScalingFactor);
    const int Timeout = style()->flickGestureTimeout();

    FlickGestureRecognizer::instance()->setFinishThreshold(HorizontalThreshold, VerticalThreshold);
    FlickGestureRecognizer::instance()->setStartThreshold(HorizontalThreshold / 2, VerticalThreshold / 2);
    FlickGestureRecognizer::instance()->setTimeout(Timeout);
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

bool KeyButtonArea::event(QEvent *e)
{
    bool eaten = false;

    if (e->type() == QEvent::Gesture) {
        const Qt::GestureType flickGestureType = FlickGestureRecognizer::sharedGestureType();
        FlickGesture *flickGesture = static_cast<FlickGesture *>(static_cast<QGestureEvent *>(e)->gesture(flickGestureType));

        if (flickGesture) {
            handleFlickGesture(flickGesture);
            eaten = true;
        }
    }

    return eaten || MWidget::event(e);
}

void KeyButtonArea::handleFlickGesture(FlickGesture *gesture)
{
    if (InputMethodMode == M::InputMethodModeDirect) {
        return;
    }

    // Any flick gesture, complete or not, resets active keys etc.
    if (!wasGestureTriggered && (gesture->state() != Qt::NoGesture)) {
        popup->hidePopup();
        longPressTimer->stop();
        clearActiveKeys();

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
                const IKeyButton *flickedKey = keyAt(gesture->startPosition());
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

void KeyButtonArea::touchPointPressed(const QPoint &pos, int id)
{
    newestTouchPointId = id;

    // Create new TouchPointInfo structure and overwrite any previous one.
    touchPoints[id] = TouchPointInfo();
    TouchPointInfo &tpi = touchPoints[id];
    tpi.pos = pos;
    tpi.initialPos = pos;

    // Reset gesture checks.
    wasGestureTriggered = false;

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
        if (accurateMode && usePopup && (id == newestTouchPointId)) {
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

    if (wasGestureTriggered) {
        return;
    }

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
        if (usePopup
            && (id == newestTouchPointId)
            && (accurateMode || popup->isPopupVisible())) {
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
        if (tpi.fingerInsideArea && (id == newestTouchPointId)) {
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

    tpi.fingerInsideArea = false;

    if (wasGestureTriggered) {
        return;
    }

    // No more long-press triggerings, although would still be possible to make
    // with another touch point.
    longPressTimer->stop();

    // Same thing here, hide popup although otherwise could be shown on
    // another touch point.
    if (id == newestTouchPointId) {
        popup->hidePopup();
    }

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

        click(key);
    } else {
        setActiveKey(0, tpi);
    }
}

void
KeyButtonArea::unlockDeadkeys()
{
    if (activeDeadkey) {
        activeDeadkey->setSelected(false);
        activeDeadkey = 0;
        updateButtonModifiers();
    }
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

void KeyButtonArea::onThemeChangeCompleted()
{
    updateButtonGeometriesForWidth(size().width());
}

void KeyButtonArea::updateButtonGeometriesForWidth(int widthOfArea)
{
    // All main keyboards should take the whole available width so we restrict
    // the button width here. This is applied only if we have equal button width.
    int equalButtonWidth = -1; // the new width if it's same for all
    if ((buttonSizeScheme == ButtonSizeEqualExpanding)
         || (buttonSizeScheme == ButtonSizeEqualExpandingPhoneNumber)) {
        const int HorizontalSpacing = style()->spacingHorizontal();
        const int MaxButtonWidth = qRound(static_cast<qreal>(widthOfArea + HorizontalSpacing)
                                          / static_cast<qreal>(section->maxColumns())
                                          - HorizontalSpacing);

        equalButtonWidth = MaxButtonWidth;
    }
    updateButtonGeometries(widthOfArea, equalButtonWidth);
}

KeyButtonArea::TouchPointInfo::TouchPointInfo()
    : fingerInsideArea(false),
      activeKey(0),
      initialKey(0),
      initialPos(),
      pos()
{
}
