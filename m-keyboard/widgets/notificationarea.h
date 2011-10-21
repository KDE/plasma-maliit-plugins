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

#ifndef NOTIFICATIONAREA_H
#define NOTIFICATIONAREA_H

#include "notificationareastyle.h"
#include "pangesture.h"

#include <MStylableWidget>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

class Notification;
class OutgoingNotificationPanParameters;
class IncomingNotificationPanParameters;
class AssistantNotificationPanParameters;

/*!
 * \class NotificationArea
 * \brief NotificationArea is used to display textual notifications during switching layouts.
 *
 *  Using this class to show there textual notifications (outgoing, incoming and assistant) on top of the virtual keyboard during switching layouts
 */
class NotificationArea : public MStylableWidget
{
    Q_OBJECT

public:

    /*!
     * \brief Constructor for creating notification area object.
     * \param parent QGraphicsItem.
     */
    explicit NotificationArea(QGraphicsItem *parent);

    //! Destructor
    ~NotificationArea();

    //! Prepares notifications for the pan gesture of \a direction.
    void prepareNotifications(PanGesture::PanDirection direction);

    //! Sets title of the outgoing layout.
    void setOutgoingLayoutTitle(const QString &title);

    //! Returns outgoing layout title.
    QString outgoingLayoutTitle() const;

    /*!
     * Sets the title of incoming layout for the pan gesture of \a diection.
     */
    void setIncomingLayoutTitle(PanGesture::PanDirection direction,
                                const QString &title);

    //! Returns the title of incoming layout for pan gesture of \a diection.
    QString setIncomingLayoutTitle(PanGesture::PanDirection direction) const;

    //! Plays show animation.
    void playShowAnimation();

    //! Plays hide animation for pan gesture of \a direction.
    void playHideAnimation(PanGesture::PanDirection direction);

    //! Cancels the notification area (show/hide) animation and hides it.
    void cancel();

    //! Sets current pan progress.
    void setProgress(qreal progress);

    /*! 
     * \brief Requires linear transition between \a startProgress and \a endProgress.
     *
     * When requring linear transition, the mutation will be disabled, and the
     * scale and opacity will be changed in linear during \a startProgress and
     * \a endProgress,
     */
    void requireLinearTransition(qreal startProgress, qreal endProgress);

    /*!
     * \brief Clears the linear transition requirements.
     */
    void clearLinearTransition();

protected:
    //! \reimp
    virtual void applyStyle();
    //! \reimp_end

private slots:
    void onNotificationAnimationFinished();

    void initPanParameters();
    void reset();

private:
    M_STYLABLE_WIDGET(NotificationAreaStyle)

    QString mOutgoingLayoutTitle;
    QString mLeftLayoutTitle;
    QString mRightLayoutTitle;
    Notification *outgoingNotification;
    Notification *incomingNotification;
    Notification *assistantNotification;
    OutgoingNotificationPanParameters *outgoingNotificationParameters;
    IncomingNotificationPanParameters *incomingNotificationParameters;
    AssistantNotificationPanParameters *assistantNotificationParameters;
    QParallelAnimationGroup hideAnimationGroup;
    QPropertyAnimation showAnimation;
    PanGesture::PanDirection direction;
    bool mMutationEnabled;

    qreal outgoingNotificationFromScale;
    qreal outgoingNotificationToScale;
    QPointF outgoingNotificationFromPos;
    QPointF outgoingNotificationToPos;
    qreal incomingNotificationFromScale;
    qreal incomingNotificationToScale;
    QPointF incomingNotificationFromPos;
    QPointF incomingNotificationToPos;
    qreal assistantNotificationFromScale;
    qreal assistantNotificationToScale;
    QPointF assistantNotificationFromPos;
    QPointF assistantNotificationToPos;
    qreal linearTransitionStartProgress;
    qreal linearTransitionEndProgress;

#ifdef UNIT_TEST
    friend class Ut_NotificationArea;
#endif
    friend class MImPanningSwitcher;
};

#endif
