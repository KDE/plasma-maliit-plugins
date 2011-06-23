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

#ifndef MIMABSTRACTKEYAREA_P_H
#define MIMABSTRACTKEYAREA_P_H

#include "keyevent.h"
#include "layoutdata.h"

#include <MFeedback>

#include <QTouchEvent>
#include <QTimer>
#include <QTime>

struct KeyContext;
class MImAbstractKey;
class FlickGesture;
class PanGesture;
class MImAbstractPopup;
class QGraphicsSceneMouseEvent;

//! \internal
class MImAbstractKeyAreaPrivate
{
public:
    Q_DECLARE_PUBLIC(MImAbstractKeyArea)

    enum MouseEventToTouchPointOption {
        CopyAllMembers,     //!< Copy all relevant members from mouse event to touch point
        ResetLastPosMember  //!< Do not use mouse event's lastPos member. Use current pos instead.
    };


    class TouchPointRecord
    {
    public:
        TouchPointRecord();

        void setHitKey(MImAbstractKey *key);

        MImAbstractKey *key() const;
        MImAbstractKey *previousKey() const;

        bool touchPointEnteredKey() const;
        bool touchPointLeftKey() const;
        bool hasGravity() const;

    private:
        MImAbstractKey *m_key;
        MImAbstractKey *m_previousKey;
        bool keyHasGravity;
    };

    //! \brief Constructor
    //! \param newSection Section that is shown by this key area
    //! \param owner Pointer to key area which owns this object
    MImAbstractKeyAreaPrivate(const LayoutData::SharedLayoutSection &newSection,
                              MImAbstractKeyArea *owner);

    //! \brief Destructor
    virtual ~MImAbstractKeyAreaPrivate();

    //! \brief Handler for flick gestures from Qt gesture framework.
    //! \param direction Gesture direction. Should have one of FlickGesture::Direction members as value.
    //! \param state gesture state
    void handleFlickGesture(int direction,
                            Qt::GestureState state);

    void handleGesture(const PanGesture &gesture);

    //! \brief Handler for touch events
    void handleTouchEvent(QTouchEvent *event);

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
    //! \param event Mouse event to create touch point from.
    //! \param option Controls which members are used. By default all members are used.
    static QTouchEvent::TouchPoint fromMouseEvent(const QGraphicsSceneMouseEvent *event,
                                                  MouseEventToTouchPointOption option = CopyAllMembers);

    //! \brief Caching GConf value for multitouch setting.
    static bool multiTouchEnabled();

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
    //!        mapped to keyarea space.
    //! \param gravityKey The key that is considered having gravity.
    //! \returns gravityKey itself or result from \a keyAt.
    MImAbstractKey *gravitationalKeyAt(const QPoint &mappedPos,
                                       MImAbstractKey *gravityKey) const;

    //! \brief Trigger a keyClicked signal, and update key area state.
    //! \param key the clicked key
    //! \param keyContext Context information for clicked key
    void click(MImAbstractKey *key, const KeyContext &keyContext);

    //! \brief Checks for speed typing mode
    //! \warning Not side-effect free when \a restartTimers is actively used.
    bool isInSpeedTypingMode(bool restartTimers = false);

    //! Switch style mode.
    void switchStyleMode();

    //! \brief Returns true if key area shows capitalazed charactes.
    bool isUpperCase() const
    {
        return (currentLevel % 2);
    }

    //! \brief Get number of rows in this key area.
    int rowCount() const
    {
        return section->rowCount();
    }

    void cancelAllKeys();

    MImAbstractKeyArea * const q_ptr;

    int currentLevel; //!< current level
    MImAbstractPopup *popup; //!< popup to show additional information for a button
    QList<QStringList> punctuationsLabels; //!< list of punctuation labels
    QList<QStringList> accentLabels; //!< list of accent labels
    bool wasGestureTriggered; //!< whether a gesture was already triggered for any active touch point
    MFeedback feedbackSliding; //!< Sliding feedback
    const LayoutData::SharedLayoutSection section; //!< layout section shown by this key area
    static M::InputMethodMode InputMethodMode; //!< used input method mode (same for all key areas)
    QTimer longPressTimer; //!< used to recognize long press
    QTimer idleVkbTimer;  //!< Whenever this key area of the VKB idles, gestures are activated.
    QTime lastTouchPointPressEvent; //!< measures elapsed time between two touchpoint press events
    bool allowedHorizontalFlick; //!< Contains true if horizontal gestures should be allowed.
    bool ignoreTouchEventsUntilNewBegin; //!< Workaround for NB#248227.
    QMap<int, QPointF> mostRecentTouchPositions; //!< Remember the most recent positions for each touch points.
    int longPressTouchPointId; //!< Remember the touch position ID which initiated long press.
    bool longPressTouchPointIsPrimary; //!< Remember if long press touch position was primary.
    QMap<int, TouchPointRecord> touchPointRecords; //!< Maps touch points to related key information.
    KeyEvent::Source source; //!< The source added to events from this area.
    bool enabledPanning;
};
//! \internal_end

#endif

