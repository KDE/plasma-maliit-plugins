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
#ifndef NOTIFICATIONAREASTYLE_H
#define NOTIFICATIONAREASTYLE_H

#include <QObject>

#include <MWidgetStyle>
#include <QEasingCurve>

/*!
    \brief Style for language change notification
*/
class M_EXPORT NotificationAreaStyle : public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(NotificationAreaStyle)

    M_STYLE_ATTRIBUTE(qreal, initialOpacity, InitialOpacity)
    M_STYLE_ATTRIBUTE(qreal, visibleOpacity, VisibleOpacity)

    M_STYLE_ATTRIBUTE(qreal, notificationMaximumWidth, NotificationMaximumWidth)
    M_STYLE_ATTRIBUTE(qreal, smallSizeScaleFactor, SmallSizeScaleFactor)
    M_STYLE_ATTRIBUTE(qreal, initialEdgeMaximumVisibleWidth, InitialEdgeMaximumVisibleWidth)
    M_STYLE_ATTRIBUTE(qreal, initialEdgeOpacity, InitialEdgeOpacity)

    M_STYLE_ATTRIBUTE(qreal, outgoingPositionStartProgress, OutgoingPositionStartProgress)
    M_STYLE_ATTRIBUTE(qreal, outgoingPositionEndProgress, OutgoingPositionEndProgress)
    M_STYLE_ATTRIBUTE(qreal, outgoingScaleStartProgress, OutgoingScaleStartProgress)
    M_STYLE_ATTRIBUTE(qreal, outgoingScaleEndProgress, OutgoingScaleEndProgress)
    M_STYLE_ATTRIBUTE(qreal, outgoingOpacityStartProgress, OutgoingOpacityStartProgress)
    M_STYLE_ATTRIBUTE(qreal, outgoingOpacityEndProgress, OutgoingOpacityEndProgress)

    M_STYLE_ATTRIBUTE(qreal, incomingPositionStartProgress, IncomingPositionStartProgress)
    M_STYLE_ATTRIBUTE(qreal, incomingPositionEndProgress, IncomingPositionEndProgress)
    M_STYLE_ATTRIBUTE(qreal, incomingScaleStartProgress, IncomingScaleStartProgress)
    M_STYLE_ATTRIBUTE(qreal, incomingScaleEndProgress, IncomingScaleEndProgress)
    M_STYLE_ATTRIBUTE(qreal, incomingOpacityStartProgress, IncomingOpacityStartProgress)
    M_STYLE_ATTRIBUTE(qreal, incomingOpacityEndProgress, IncomingOpacityEndProgress)

    M_STYLE_ATTRIBUTE(qreal, outgoingScaleMutationProgress, OutgoingScaleMutationProgress)
    M_STYLE_ATTRIBUTE(qreal, outgoingScaleMutation, OutgoingScaleMutation)
    M_STYLE_ATTRIBUTE(qreal, outgoingOpacityMutationProgress, OutgoingOpacityMutationProgress)
    M_STYLE_ATTRIBUTE(qreal, outgoingOpacityMutation, OutgoingOpacityMutation)
    M_STYLE_ATTRIBUTE(qreal, incomingScaleMutationProgress, IncomingScaleMutationProgress)
    M_STYLE_ATTRIBUTE(qreal, incomingScaleMutation, IncomingScaleMutation)
    M_STYLE_ATTRIBUTE(qreal, incomingOpacityMutationProgress, IncomingOpacityMutationProgress)
    M_STYLE_ATTRIBUTE(qreal, incomingOpacityMutation, IncomingOpacityMutation)

    M_STYLE_ATTRIBUTE(QEasingCurve, showAnimationCurve, ShowAnimationCurve)
    M_STYLE_ATTRIBUTE(int, showAnimationDuration, ShowAnimationDuration)

    M_STYLE_ATTRIBUTE(QEasingCurve, hideAnimationCurve, HideAnimationCurve)
    M_STYLE_ATTRIBUTE(int, hideAnimationDuration, HideAnimationDuration)
};

class M_EXPORT NotificationAreaStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(NotificationAreaStyle)
};

#endif // NOTIFICATIONSTYLE_H
