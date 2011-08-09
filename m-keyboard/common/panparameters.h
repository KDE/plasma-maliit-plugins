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

#ifndef PANNINGPARAMETERS_H
#define PANNINGPARAMETERS_H

#include <QObject>
#include <QRectF>
#include <QPointF>

/*!
 * \brief Helper class responsible for generating position, scale and opacity value.
 *
 *  Implementor must reimepement the update(), according the input progress to
 *  generate position, scale and opacity value.
 */
class PanParameters : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PanParameters)
public:
    PanParameters(QObject *parent = 0);

    virtual ~PanParameters();

    /*!
     *  \brief progress is value from 0.0 to 1.0.
     */
    void setProgress(qreal progress);

    //! Returns current progress.
    qreal progress() const;

    //! Sets the opacity factor
    void setOpacityFactor(qreal factor);

    //! Returns the opacity factor, default opacity factor is 1.0
    qreal opacityFactor() const;

    //! Returns the opacity.
    qreal opacity() const;

    //! Returns the position.
    QPointF position() const;

    //! Returns the scale.
    qreal scale() const;

    //! Sets the change range for position.
    void setPositionRange(QPointF fromPosition, QPointF toPosition);

    //! Sets the change range for opacity.
    void setOpacityRange(qreal fromOpacity, qreal toOpacity);

    //! Sets the change range for scale.
    void setScaleRange(qreal fromScale, qreal toScale);

    /*!
     * Sets the start progress for position at which it will begin to change,
     * and the end progress it finish changing.
     */
    void setPositionProgressRange(qreal startProgress, qreal endProgress);

    /*!
     * Sets the start progress for scale at which it will begin to change,
     * and the end progress it finish changing.
     */
    void setScaleProgressRange(qreal startProgress, qreal endProgress);

    /*!
     * Sets the start progress for opacity at which it will begin to change,
     * and the end progress it finish changing.
     */
    void setOpacityProgressRange(qreal startProgress, qreal endProgress);

    /*!
     * Reimplement this method according the input progress
     * to generate position, scale and opacity value.
     */
    virtual void update() = 0;

    /*!
     * Reimplement this method to reset.
     */
    virtual void reset() = 0;

signals:
    //! Emitted when position is changed to \a pos.
    void positionChanged(const QPointF &pos);

    //! Emitted when opacity is changed to \a opacity.
    void opacityChanged(qreal opacity);

    //! Emitted when scale is changed to \a scale.
    void scaleChanged(qreal scale);

protected:
    qreal mProgress;
    qreal mOpacityFactor;
    QPointF mFromPosition;
    QPointF mToPosition;
    qreal mFromOpacity;
    qreal mToOpacity;
    qreal mFromScale;
    qreal mToScale;
    QPointF mPosition;
    qreal mOpacity;
    qreal mScale;
    qreal mPositionStartProgress;
    qreal mPositionEndProgress;
    qreal mScaleStartProgress;
    qreal mScaleEndProgress;
    qreal mOpacityStartProgress;
    qreal mOpacityEndProgress;
};

#endif
