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

#ifndef MIMABSTRACTKEYAREA_H
#define MIMABSTRACTKEYAREA_H

#include "mkeyboardcommon.h"
#include "mimabstractkey.h"
#include "mimabstractkeyareastyle.h"
#include "layoutdata.h"

#include <MStylableWidget>
#include <MFeedback>
#include <QList>
#include <QStringList>
#include <QTouchEvent>
#include <QTimer>
#include <QTime>

class FlickGesture;
class MReactionMap;
class PopupBase;

//! \brief MImAbstractKeyArea is a view for virtual keyboard layout represented by LayoutModel
class MImAbstractKeyArea
    : public MStylableWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(MImAbstractKeyArea)

public:
    //! \brief Constructor
    //! \param section section that is shown by this key area
    //! \param usePopup whether popup should be used
    //! \param parent key area's parent
    explicit MImAbstractKeyArea(const LayoutData::SharedLayoutSection &section,
                                bool usePopup = false,
                                QGraphicsWidget *parent = 0);

    //! \brief Destructor
    virtual ~MImAbstractKeyArea();

    //! \brief Returns section shown by this key area.
    const LayoutData::SharedLayoutSection &sectionModel() const;

    //! \brief Returns current level of this layout.
    int level() const;

    //! \brief Exposes style used by this key area.
    const MImAbstractKeyAreaStyleContainer &baseStyle() const;

    //! \brief Sets input method mode for all MImAbstractKeyArea instances.
    //! \param inputMethodMode the new input method mode
    static void setInputMethodMode(M::InputMethodMode inputMethodMode);

    //! \brief Returns relative button base width
    qreal relativeKeyBaseWidth() const;

    //! \brief Returns all keys from this key area.
    virtual QList<const MImAbstractKey *> keys() const = 0;

    //! \brief Notification for derived classes about button modifier change.
    //!
    //! Derived classes should not change the level of selected dead keys. This is to
    //! ensure all dead keys can be used with all characters in every level.
    //! \param shift whether shift modifier is enabled
    //! \param accent which accented version should be used, for a key
    virtual void modifiersChanged(bool shift,
                                  const QChar &accent = QChar());

public slots:
    //! \brief Tell key area to switch levels for all keys.
    //! \param level the new level
    void switchLevel(int level);

    //! \brief Set shift state.
    //! \param shiftState the new shift state
    virtual void setShiftState(ModifierState shiftState);

    //! \brief Draw reactive areas for all keys.
    //! \param reactionMap the reaction map to draw onto
    //! \param view the view to be used
    virtual void drawReactiveAreas(MReactionMap *reactionMap,
                                   QGraphicsView *view);

    //! \brief Unlock all locked dead keys.
    //! \param deadKey the corresponding dead key
    void unlockDeadKeys(MImAbstractKey *deadKey);

    //! \brief Hide popup
    void hidePopup();

signals:
    //! \brief Emitted when key is pressed
    //!
    //! Note that this happens also when user keeps finger down/mouse
    //! button pressed and moves over another key (event is about the new key)
    //! \param key describes pressed button
    //! \param accent label of pressed dead key if any
    //! \param upperCase contains true if key is in uppercase state
    void keyPressed(const MImAbstractKey *key,
                    const QString &accent,
                    bool upperCase);

    //! \brief Emitted when key is released.
    //!
    //! Note that this happens also when user keeps finger down/mouse
    //! button pressed and moves over another key (event is about the old key)
    //! \param key describes released button
    //! \param accent label of pressed dead key if any
    //! \param upperCase contains true if key is in uppercase state
    void keyReleased(const MImAbstractKey *key,
                     const QString &accent,
                     bool upperCase);

    //! \brief Emitted when user releases mouse button/lifts finger.
    //!
    //! Except when done on a dead key
    //! \param key describes clicked button
    //! \param accent label of pressed dead key if any
    //! \param upperCase contains true if key is in uppercase state
    //! \param touchPoint the touch point for the key
    void keyClicked(const MImAbstractKey *key,
                    const QString &accent, bool upperCase,
                    const QPoint &touchPoint);

    //! \brief Emitted when long press is detected.
    //!
    //! Long press detection is:
    //! - cancelled when latest pressed key is released;
    //! - restarted when finger is moved to other key;
    //! - restarted when new touch point is recognized by MImAbstractKeyArea.
    //! \param key describes pressed button
    //! \param accent active accent in MImAbstractKeyArea at the time of long press occured
    //! \param upperCase upper case state in MImAbstractKeyArea at the time of long press occured
    void longKeyPressed(const MImAbstractKey *key,
                        const QString &accent,
                        bool upperCase);

    //! \brief Emitted when key area is flicked right.
    void flickRight();

    //! \brief Emitted when key area is flicked left.
    void flickLeft();

    //! \brief Emitted when key area is flicked down.
    void flickDown();

    //! \brief Emitted when key area is flicked up.
    //! \param binding Information about the key where mouse button was pressed
    void flickUp(const MImKeyBinding &binding);

    //! \brief Emitted if button width has changed
    //! \param baseWidth base width used for relative button widths
    void relativeKeyBaseWidthChanged(qreal baseWidth);

protected:
    //! \reimp
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change,
                                const QVariant &value);

    virtual void grabMouseEvent(QEvent *event);
    virtual void ungrabMouseEvent(QEvent *event);
    virtual bool event(QEvent *event);
    //! \reimp_end

    //! \brief Called when key area's visibility changed.
    //! \param visible the new visbility status
    virtual void handleVisibilityChanged(bool visible);

    //! \brief Invalidates the current background cache.
    virtual void invalidateBackgroundCache() = 0;

    //! Shows popup and updates its content and position.
    //! \param key current key
    void updatePopup(MImAbstractKey *key = 0);

    //! \brief Get maximum number of columns in this key area.
    int maxColumns() const;

    //! \brief Get number of rows in this key area.
    int rowCount() const;

    //! \brief Updates button labels and/or icons according to current level
    //!        and deadkey.
    //! \param accent the accent version of a dead key.
    void updateKeyModifiers(const QChar &accent = QChar());

    //! \brief Returns key at given \a pos.
    //!
    //! Accepts positions outside widget geometry because
    //! of reactive margins.
    //! \param pos the position (in key area space) to look up key
    virtual MImAbstractKey *keyAt(const QPoint &pos) const = 0;

    //! \brief Updates key (and row) geometry based on given \a availableWidth.
    //! \param availableWidth with of the key area
    virtual void updateKeyGeometries(int availableWidth) = 0;

    //! \brief Returns popup
    const PopupBase &popup() const;

    //! \brief Log touch point information to
    //!        $HOME/.meego-im/vkb-touchpoints.log, for debugging purposes.
    //! \param tp the touch point
    //! \param key key underneath touch point
    //! \param lastKey last key that was associated to the touch point
    void logTouchPoint(const QTouchEvent::TouchPoint &tp,
                       const MImAbstractKey *key,
                       const MImAbstractKey *lastKey = 0) const;

    qreal mRelativeKeyBaseWidth; //!< Relative key base width in currently active layout
    bool debugTouchPoints; //!< Whether touch point debugging is enabled

protected slots:
    //! Update background images, text layouts, etc. when the theme changed.
    virtual void onThemeChangeCompleted();

    //! Handle long press on the key
    virtual void handleLongKeyPressed();

    //! Handle idle VKB
    virtual void handleIdleVkb();

private:
    //! \brief Handler for flick gestures from Qt gesture framework.
    //! \param gesture the flick gesture
    void handleFlickGesture(FlickGesture *gesture);

    //! \brief Touch point press handler.
    //! \param tp The unprocessed Qt touchpoint.
    void touchPointPressed(const QTouchEvent::TouchPoint &tp);

    //! \brief Touch point move handler.
    //! \param tp The unprocessed Qt touchpoint.
    void touchPointMoved(const QTouchEvent::TouchPoint &tp);

    //! \brief Touch point release handler.
    //! \param tp The unprocessed Qt touchpoint.
    void touchPointReleased(const QTouchEvent::TouchPoint &tp);

    //! \brief Helper method to create touch points
    //! \param id touch point id
    //! \param state touch point state
    //! \param pos touch point scene position
    //! \param lastPos last touch point scene position
    static QTouchEvent::TouchPoint createTouchPoint(int id,
                                                    Qt::TouchPointState state,
                                                    const QPointF &pos,
                                                    const QPointF &lastPos);

    //! \brief Helper struct to store results of \a gravitationalKeyAt
    struct GravitationalLookupResult
    {
        GravitationalLookupResult(MImAbstractKey *newKey,
                                  MImAbstractKey *newLastKey)
            : key(newKey)
            , lastKey(newLastKey)
        {}

        MImAbstractKey *key;
        MImAbstractKey *lastKey;
    };

    //! \brief Gravitational key lookup
    //! \param mappedPos Current position of touchpoint,
    //!        mapped to widget space.
    //! \param mappedLastPos Last position of touchpoint,
    //!        mapped to widget space.
    //! \returns adjusted key and last key, using \a keyAt.
    GravitationalLookupResult gravitationalKeyAt(const QPoint &mappedPos,
                                                 const QPoint &mappedLastPos) const;

    //! \brief Trigger a keyClicked signal, and update key area state.
    //! \param key the clicked key
    //! \param pos where the key was clicked
    void click(MImAbstractKey *key,
               const QPoint &pos = QPoint());

    //! \brief Checks for speed typing mode
    //! \warning Not side-effect free when \a restartTimers is actively used.
    bool isInSpeedTypingMode(bool restartTimers = false);

    //! Switch style mode.
    void switchStyleMode();

    int currentLevel; //!< current level
    PopupBase *mPopup; //!< popup to show additional information for a button
    QList<QStringList> punctuationsLabels; //!< list of punctuation labels
    QList<QStringList> accentLabels; //!< list of accent labels
    bool wasGestureTriggered; //!< whether a gesture was already triggered for any active touch point
    bool enableMultiTouch; //!< whether this key area operates in multitouch mode
    MFeedback feedbackPress; //!< Press feedback
    MFeedback feedbackCancel; //!< Cancel feedback
    const LayoutData::SharedLayoutSection section; //!< layout section shown by this key area
    static M::InputMethodMode InputMethodMode; //!< used input method mode (same for all key areas)
    QTimer longPressTimer; //!< used to recognize long press
    QTimer idleVkbTimer;  //!< Whenever this key area of the VKB idles, gestures are activated.
    QTime lastTouchPointPressEvent; //!< measures elapsed time between two touchpoint press events

    M_STYLABLE_WIDGET(MImAbstractKeyAreaStyle)

#ifdef UNIT_TEST
    friend class MReactionMapTester;
    friend class Ut_MImAbstractKeyArea;
    friend class Ut_SymbolView;
    friend class Ut_MVirtualKeyboard;
    friend class Bm_MImAbstractKeyArea; //benchmarks
    friend class Bm_Painting;
#endif
};

#endif
