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
#ifndef TOUCHFORWARDFILTER_H
#define TOUCHFORWARDFILTER_H

#include <QEvent>
#include <QObject>
#include <QPointer>
#include <QTouchEvent>
class QGraphicsObject;

/*!
 *  \brief Forwards touch events from one QGraphicsObject to another.
 *
 *  The target is guaranteed to get touch events in proper order, that is:
 *  TouchBegin, [TouchUpdate, ...], TouchEnd.
 *  Touch points in forwarded events and directly sent events are not merged
 *  but touch point ids of forwarded events are adjusted so that they do not
 *  overlap.
 *
 *  Example, R = receiver widget, F = filtered widget
 *  -# Sent to R: TouchBegin
 *     - id=0, pressed
 *  -# Arrives (at R): TouchBegin
 *     - id=0, pressed
 *  -# Sent to F: TouchBegin
 *     - id=0, pressed
 *     - id=1, pressed
 *  -# Arrives: TouchUpdate, only contains updates for F
 *     - id=<unique id 1>, pressed
 *     - id=<unique id 2>, pressed
 *  -# Sent to R: TouchUpdate
 *     - id=0, moved
 *  -# Arrives: TouchUpdate, only contains updates for R
 *     - id=0, moved
 *  -# Sent to R: TouchEnd
 *     - id=0, released
 *  -# Arrives: TouchUpdate
 *     - id=0, released
 *  -# Sent to F: TouchEnd
 *     - id=0, released
 *     - id=1, released
 *  -# Arrives: TouchEnd
 *     - id=<unique id 1>, released
 *     - id=<unique id 2>, released
 *
 *
 *  TouchForwardFilter is implemented as a QObject filter. User should not install
 *  the filter, it is done automatically on creation.
 *
 *  The filter should be created on heap, like:
 *  \code
 *      (void)new TouchForwardFilter(target, initialTargetState, origin);
 *  \endcode
 *
 *  The filter will be destroyed automatically when either target is destroyed,
 *  hidden, or when filtered object is about to receive TouchEnd event.
 *  Otherwise, it is up to user to destroy the filter.
 */
class TouchForwardFilter : public QObject
{
    Q_OBJECT
public:
    enum ItemTouchState {
        TouchActive,  //!< Item has received TouchBegin but not TouchEnd.
        TouchInactive //!< Item has received TouchEnd, or no touch events at all.
    };

    enum TouchPointConversionOption {
        NoOption = 0x00,
        ConvertToPress = 0x01,
        NoPress = 0x02,
        DiscardLastPosition = 0x04
    };
    Q_DECLARE_FLAGS(TouchPointConversionOptions, TouchPointConversionOption)

    //! \brief Constructs a filter that will forward touch events to a target.
    //! \param target QGraphicsObject to who the forwarded events will be sent to.
    //! \param initialTargetTouchState Whether \a target is already receiving touch events.
    //!                                New target events will be monitored automatically.
    //! \param eventOrigin The source of events to be forwarded.
    //! \param initialTouchEvent If given, this event will be converted and sent to target immediately.
    explicit TouchForwardFilter(QGraphicsObject *target,
                                ItemTouchState initialTargetTouchState,
                                QGraphicsObject *eventOrigin,
                                const QTouchEvent *initialTouchEvent = 0);
    virtual ~TouchForwardFilter();

    //! \brief Uses original id and QGraphicsObject, who received the touch point, to make a unique id.
    static int uniqueTouchId(const QGraphicsObject &originItem, int id);

    static const int MaxTouchPointId;

protected:
    //! \reimp
    virtual bool eventFilter(QObject *watched, QEvent *event);
    //! \reimp_end

private:
    bool handleTouchEventFromOrigin(const QGraphicsObject &originItem,
                                    const QTouchEvent &touchEvent);
    bool handleTouchEventFromTarget(QTouchEvent &touchEvent);

    void sendEvent(const QTouchEvent &event);

    QTouchEvent convertTouchEvent(const QTouchEvent &touchEvent,
                                  QEvent::Type overriddenType,
                                  const QGraphicsObject &originItem,
                                  TouchPointConversionOptions touchPointOptions) const;

    QTouchEvent::TouchPoint convertTouchPoint(const QTouchEvent::TouchPoint &touchPoint,
                                              const QGraphicsObject &originItem,
                                              TouchPointConversionOptions options) const;

private slots:
    void deleteLaterIfTargetHidden();

private:
    QPointer<QGraphicsObject> target;
    bool isFirstOriginEvent;
    ItemTouchState originTouchState;
    ItemTouchState targetTouchState;
};

#endif // TOUCHFORWARDFILTER_H
