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

#ifndef NOTIFICATIONPANNINGPARAMETERS_H
#define NOTIFICATIONPANNINGPARAMETERS_H

#include "panparameters.h"
#include "pangesture.h"

class NotificationPanParameters : public PanParameters
{
public:
    NotificationPanParameters(QObject *parent = 0)
        : PanParameters(parent),
          scaleMutationProgress(0.0),
          scaleMutation(1.0),
          opacityMutationProgress(0.0),
          opacityMutation(1.0)
    {
    };

    /*!
     * \brief Set a mutation jump to \a scale at \a progress. 
     */
    virtual void setScaleMutation(qreal progress, qreal scale)
    {
        // check whether the input progress and scale are valid
        if (progress > 0.0001
            && progress >= mScaleStartProgress
            && progress <= mScaleEndProgress
            && ((scale >= mFromScale && scale <= mToScale)
                || (scale <= mFromScale && scale >= mToScale))) {
            scaleMutationProgress = progress;
            scaleMutation = scale;
        } else {
            scaleMutationProgress = 0.0;
            scaleMutation = 0.0;
        }
    };

    /*!
     * \brief Set a mutation jump to \a opacity at \a progress. 
     */
    virtual void setOpacityMutation(qreal progress, qreal opacity)
    {
        // check whether the input progress and opacity are valid
        if (progress > 0.0001
            && progress >= mOpacityStartProgress
            && progress <= mOpacityEndProgress
            && ((opacity >= mFromOpacity && opacity <= mToOpacity)
                || (opacity <= mFromOpacity && opacity >= mToOpacity))) {
            opacityMutationProgress = progress;
            opacityMutation = opacity;
        } else {
            opacityMutationProgress = 0.0;
            opacityMutation = 0.0;
        }
    };

    //! Returns the scale at \a progress
    virtual qreal scaleAt(qreal progress) const = 0;

    //! Returns the opacity at \a progress
    virtual qreal opacityAt(qreal progress) const = 0;

    //! Returns the position at \a progress
    virtual QPointF positionAt(qreal progress) const = 0;

protected:
    qreal scaleMutationProgress;
    qreal scaleMutation;
    qreal opacityMutationProgress;
    qreal opacityMutation;
};

class IncomingNotificationPanParameters : public NotificationPanParameters
{
public:
    IncomingNotificationPanParameters(QObject *parent = 0)
        : NotificationPanParameters(parent)
    {
    };

    //! reimp
    virtual qreal scaleAt(qreal progress) const
    {
        qreal scale = mScale;
        if (scaleMutationProgress > 0.0001
            && progress >= scaleMutationProgress) {
            // sudden change for scale
            scale = scaleMutation
                    + (mToScale - scaleMutation)
                    * qBound<qreal>(0,
                                    (progress - scaleMutationProgress) /
                                    (mScaleEndProgress - scaleMutationProgress),
                                    1.0);
        } else {
            scale = mFromScale
                    + (mToScale - mFromScale)
                    * qBound<qreal>(0,
                                    (progress - mScaleStartProgress)
                                    / (mScaleEndProgress - mScaleStartProgress),
                                    1.0);
        }
        return scale;
    };

    virtual qreal opacityAt(qreal progress) const
    {
        qreal opacity = mOpacity;
        if (opacityMutationProgress > 0.0001
            && progress >= opacityMutationProgress) {
            // sudden change for opacity
            opacity = opacityMutation
                      + (mToOpacity - opacityMutation)
                      * qBound<qreal>(0,
                                      (progress - opacityMutationProgress)
                                       / (mOpacityEndProgress - opacityMutationProgress),
                                       1.0);
        } else {
            opacity = mFromOpacity
                      + (mToOpacity - mFromOpacity)
                      * qBound<qreal>(0,
                                      (progress - mOpacityStartProgress)
                                      / (mOpacityEndProgress - mOpacityStartProgress),
                                      1.0);
        }
        return opacity;
    };

    virtual QPointF positionAt(qreal progress) const
    {
        qreal scale = scaleAt(progress);
        QPointF position = mPosition;
        position = mFromPosition
                   + QPointF(mToPosition - mFromPosition)
                   * qBound<qreal>(0,
                                   (progress - mPositionStartProgress) /
                                   (mPositionEndProgress - mPositionStartProgress),
                                   1.0);

        // postion is always align bottom
        const qreal posYMovement = QPointF(mToPosition - mFromPosition).y();
        position.setY(mFromPosition.y()
                      + qAbs<qreal>(scale - mFromScale)
                      / qAbs<qreal>(mToScale - mFromScale)
                      * posYMovement);
        return position;
    };

    virtual void update() {
        mScale = scaleAt(mProgress);
        mOpacity = opacityAt(mProgress);
        mPosition = positionAt(mProgress);
    };

    virtual void reset()
    {
        mProgress = 0;
        mOpacity = mFromOpacity;
        mScale = mFromScale;
        mPosition = mFromPosition;
    };
    //! reimp_end
};

class OutgoingNotificationPanParameters : public NotificationPanParameters
{
public:
    OutgoingNotificationPanParameters(QObject *parent = 0)
        : NotificationPanParameters(parent),
          avoidOverlappingPanParameters(0),
          direction(PanGesture::PanNone),
          itemWidth(0.0)
    {
    };

    //! reimp
    virtual qreal scaleAt(qreal progress) const
    {
        qreal scale = mScale;
        if (scaleMutationProgress > 0.0001
            && progress >= scaleMutationProgress) {
            // sudden change for scale
            scale = scaleMutation
                    + (mToScale - scaleMutation)
                    * qBound<qreal>(0,
                                    (progress - scaleMutationProgress) /
                                    (mScaleEndProgress - scaleMutationProgress),
                                    1.0);
        } else {
            scale = mFromScale
                    + (mToScale - mFromScale)
                    * qBound<qreal>(0,
                                    (progress - mScaleStartProgress)
                                    / (mScaleEndProgress - mScaleStartProgress),
                                    1.0);
        }
        return scale;
    };

    virtual qreal opacityAt(qreal progress) const
    {
        qreal opacity = mOpacity;
        if (opacityMutationProgress > 0.0001
            && progress >= opacityMutationProgress) {
            // sudden change for opacity
            opacity = opacityMutation
                      + (mToOpacity - opacityMutation)
                      * qBound<qreal>(0,
                                    (progress - opacityMutationProgress) /
                                    (mOpacityEndProgress - opacityMutationProgress),
                                    1.0);
        } else {
            opacity = mFromOpacity
                      + (mToOpacity - mFromOpacity)
                      * qBound<qreal>(0,
                                      (progress - mOpacityStartProgress)
                                      / (mOpacityEndProgress - mOpacityStartProgress),
                                      1.0);
        }
        return opacity;
    };

    virtual QPointF positionAt(qreal progress) const
    {
        qreal scale = scaleAt(progress);
        QPointF position = mPosition;
        position = mFromPosition
                   + QPointF(mToPosition - mFromPosition)
                   * qBound<qreal>(0,
                                   (progress - mPositionStartProgress) /
                                   (mPositionEndProgress - mPositionStartProgress),
                                   1.0);

        if (avoidOverlappingPanParameters) {
            QPointF avoidOverlappingItemPos =
                avoidOverlappingPanParameters->positionAt(progress);
            qreal avoidOverlappingItemScale =
                avoidOverlappingPanParameters->scaleAt(progress);
            if (direction == PanGesture::PanLeft
                && (position.x() + itemWidth * scale
                     > avoidOverlappingItemPos.x())) {
                position.setX(avoidOverlappingItemPos.x()
                              - itemWidth * scale);
            } else if (direction == PanGesture::PanRight
                       && (position.x() < avoidOverlappingItemPos.x()
                           + itemWidth * avoidOverlappingItemScale)) {
                position.setX(avoidOverlappingItemPos.x()
                              + itemWidth * avoidOverlappingItemScale);
            }
        }

        // postion is always align bottom
        const qreal posYMovement = QPointF(mToPosition - mFromPosition).y();
        position.setY(mFromPosition.y()
                      + qAbs<qreal>(scale - mFromScale)
                       / qAbs<qreal>(mToScale - mFromScale)
                       * posYMovement);
        return position;
    };

    virtual void update() {
        mScale = scaleAt(mProgress);
        mOpacity = opacityAt(mProgress);
        mPosition = positionAt(mProgress);
    };

    virtual void reset()
    {
        mProgress = 0;
        mOpacity = mFromOpacity;
        mScale = mFromScale;
        mPosition = mFromPosition;
    };
    //! reimp_end

    /*!
     * This method is called when pan to \a direction and wants to avoid
     * overlapping with \a parameter.
     * 
     * \param parameter is the PanParameters to avoid overlapping.
     * \param direction is the panning direction.
     * \param itemWidth is the width of item between position() and the
     * position() of \a parameter. When direction is PanGesture::PanLeft, the
     * item is the outgoing notification, when it is PanGesture::PanRight, it
     * is the incoming notification.
     */
    void setAvoidOverlappingPanParameters(NotificationPanParameters *parameter,
                                      PanGesture::PanDirection direction,
                                      qreal itemWidth)
    {
        avoidOverlappingPanParameters = parameter;
        this->direction = direction;
        this->itemWidth = itemWidth;
    };

private:
    NotificationPanParameters *avoidOverlappingPanParameters;
    PanGesture::PanDirection direction;
    qreal itemWidth;
};

class AssistantNotificationPanParameters : public PanParameters
{
public:
    AssistantNotificationPanParameters(QObject *parent = 0)
        : PanParameters(parent),
          keepDistancePanParameters(0),
          direction(PanGesture::PanNone),
          itemWidth(0.0),
          distance(0.0)
    {
    };

    //! reimp
    virtual qreal scaleAt(qreal progress) const
    {
        return (mFromScale + (mToScale - mFromScale) * progress);
    };

    virtual qreal opacityAt(qreal progress) const
    {
        qreal opacity = mOpacity;
        opacity = mFromOpacity
                  + (mToOpacity - mFromOpacity)
                  * qBound<qreal>(0,
                                  (progress - mOpacityStartProgress)
                                  / (mOpacityEndProgress - mOpacityStartProgress),
                                  1.0);
        return opacity;
    };

    virtual QPointF positionAt(qreal progress) const
    {
        qreal scale = scaleAt(progress);
        QPointF position = mPosition;
        position = mFromPosition
                   + QPointF(mToPosition - mFromPosition)
                   * qBound<qreal>(0,
                                   (progress - mPositionStartProgress) /
                                   (mPositionEndProgress - mPositionStartProgress),
                                   1.0);

        if (keepDistancePanParameters) {
            QPointF keepDistanceItemPos =
                keepDistancePanParameters->positionAt(progress);
            qreal keepDistanceItemScale = keepDistancePanParameters->scaleAt(progress);
            if (direction == PanGesture::PanLeft
                && (position.x() > keepDistanceItemPos.x()
                    - itemWidth * scale - distance)) {
                position.setX(keepDistanceItemPos.x()
                              - itemWidth * scale - distance);
            } else if (direction == PanGesture::PanRight
                       && (position.x() < keepDistanceItemPos.x()
                           + distance
                           + itemWidth * keepDistanceItemScale)) {
                position.setX(keepDistanceItemPos.x()
                              + distance
                              + itemWidth * keepDistanceItemScale);
            }
        }

        // postion is always align bottom
        const qreal posYMovement = QPointF(mToPosition - mFromPosition).y();
        position.setY(mFromPosition.y()
                      + qAbs<qreal>(scale - mFromScale)
                      / qAbs<qreal>(mToScale - mFromScale)
                      * posYMovement);
        return position;
    };

    virtual void update()
    {
        mScale = scaleAt(mProgress);
        mOpacity = opacityAt(mProgress);
        mPosition = positionAt(mProgress);
    };

    virtual void reset()
    {
        mProgress = 0;
        mOpacity = mFromOpacity;
        mScale = mFromScale;
        mPosition = mFromPosition;
    };
    //! reimp_end

    /*!
     * This method is called when pan to \a direction and wants to keep
     * \a distance with \a parameter.
     * 
     * \param parameter is the PanParameters to be kept distance.
     * \param direction is the panning direction.
     * \param itemWidth is the width of item between position() and
     * the position() of \a parameter. When direction is
     * PanGesture::PanLeft, the item is the outgoing notification,
     * when it is PanGesture::PanRight, the item is the assistant
     * notification.
     * \param distance is the distance between assistant and outgoing
     * notifications.
     */
    void setKeepDistancePanParameters(NotificationPanParameters *parameter,
                                      PanGesture::PanDirection direction,
                                      qreal itemWidth,
                                      qreal distance)
    {
        keepDistancePanParameters = parameter;
        this->direction = direction;
        this->itemWidth = itemWidth;
        this->distance = distance;
    };

private:
    NotificationPanParameters *keepDistancePanParameters;
    PanGesture::PanDirection direction;
    qreal itemWidth;
    qreal distance;
};

#endif
