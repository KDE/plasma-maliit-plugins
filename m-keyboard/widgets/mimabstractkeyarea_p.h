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

#ifndef MIMABSTRACTKEYAREA_P_H
#define MIMABSTRACTKEYAREA_P_H

#include "layoutdata.h"

#include <MFeedback>

#include <QTouchEvent>
#include <QTimer>
#include <QTime>

class MImAbstractKey;
class FlickGesture;
class PopupBase;

//! \internal
class MImAbstractKeyAreaPrivate
{
public:
    Q_DECLARE_PUBLIC(MImAbstractKeyArea)

    //! \brief Constructor
    //! \param newSection Section that is shown by this key area
    //! \param owner Pointer to key area which owns this object
    MImAbstractKeyAreaPrivate(const LayoutData::SharedLayoutSection &newSection,
                              MImAbstractKeyArea *owner);

    //! \brief Destructor
    virtual ~MImAbstractKeyAreaPrivate();

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

    MImAbstractKeyArea * const q_ptr;

    int currentLevel; //!< current level
    PopupBase *mPopup; //!< popup to show additional information for a button
    QList<QStringList> punctuationsLabels; //!< list of punctuation labels
    QList<QStringList> accentLabels; //!< list of accent labels
    bool wasGestureTriggered; //!< whether a gesture was already triggered for any active touch point
    bool enableMultiTouch; //!< whether this key area operates in multitouch mode
    MFeedback feedbackSliding; //!< Sliding feedback
    const LayoutData::SharedLayoutSection section; //!< layout section shown by this key area
    static M::InputMethodMode InputMethodMode; //!< used input method mode (same for all key areas)
    QTimer longPressTimer; //!< used to recognize long press
    QTimer idleVkbTimer;  //!< Whenever this key area of the VKB idles, gestures are activated.
    QTime lastTouchPointPressEvent; //!< measures elapsed time between two touchpoint press events
};
//! \internal_end

#endif

