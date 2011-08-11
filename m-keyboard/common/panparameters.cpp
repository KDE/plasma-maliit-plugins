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

#include "panparameters.h"

PanParameters::PanParameters(QObject *parent)
    : QObject(parent),
      mProgress(0.0),
      mOpacityFactor(1.0),
      mFromOpacity(1.0),
      mToOpacity(1.0),
      mFromScale(1.0),
      mToScale(1.0),
      mOpacity(1.0),
      mScale(1.0),
      mPositionStartProgress(0.0),
      mPositionEndProgress(1.0),
      mScaleStartProgress(0.0),
      mScaleEndProgress(1.0),
      mOpacityStartProgress(0.0),
      mOpacityEndProgress(1.0)
{
}

PanParameters::~PanParameters()
{
}

void PanParameters::setProgress(qreal progress)
{
    mProgress = progress;
    QPointF oldPosition = mPosition;
    qreal oldScale = mScale;
    qreal oldOpacity = mOpacity;

    update();

    if (oldPosition != mPosition) {
        emit positionChanged(mPosition);
    }
    if (oldScale != mScale) {
        emit scaleChanged(mScale);
    }
    if (oldOpacity != mOpacity) {
        emit opacityChanged(mOpacity);
    }
}


qreal PanParameters::progress() const
{
    return mProgress;
}

void PanParameters::setOpacityFactor(qreal factor)
{
    mOpacityFactor = factor;
}

qreal PanParameters::opacityFactor() const
{
    return mOpacityFactor;
}

qreal PanParameters::opacity() const
{
    return mOpacity;
}

QPointF PanParameters::position() const
{
    return mPosition;
}

qreal PanParameters::scale() const
{
    return mScale;
}

void PanParameters::setPositionRange(QPointF fromPosition, QPointF toPosition)
{
    mFromPosition = fromPosition;
    mToPosition = toPosition;
}

void PanParameters::setOpacityRange(qreal fromOpacity, qreal toOpacity)
{
    mFromOpacity = fromOpacity;
    mToOpacity = toOpacity;
}

void PanParameters::setScaleRange(qreal fromScale, qreal toScale)
{
    mFromScale = fromScale;
    mToScale = toScale;
}

void PanParameters::setPositionProgressRange(qreal startProgress, qreal endProgress)
{
    mPositionStartProgress = startProgress;
    mPositionEndProgress = endProgress;
}

void PanParameters::setScaleProgressRange(qreal startProgress, qreal endProgress)
{
    mScaleStartProgress = startProgress;
    mScaleEndProgress = endProgress;
}

void PanParameters::setOpacityProgressRange(qreal startProgress, qreal endProgress)
{
    mOpacityStartProgress = startProgress;
    mOpacityEndProgress = endProgress;
}
