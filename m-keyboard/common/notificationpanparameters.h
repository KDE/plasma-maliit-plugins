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

class NotificationPanParameters : public PanParameters
{
public:
    NotificationPanParameters(QObject *parent = 0)
        : PanParameters(parent)
    {
    };

    //! reimp
    virtual void update() {

        mPosition = mFromPosition
            + QPointF(mToPosition - mFromPosition)
            * qBound<qreal>(0,
                    (mProgress - mPositionStartProgress) /
                    (mPositionEndProgress - mPositionStartProgress),
                    1.0);
        if (mProgress < mScaleStartProgress) {
            mPosition.setY(mFromPosition.y());
        } else {
            mPosition.setY(mFromPosition.y()
                           + QPointF(mToPosition - mFromPosition).y()
                           * qBound<qreal>(0,
                                           (mProgress - mScaleStartProgress) /
                                           (mScaleEndProgress - mScaleStartProgress),
                                           1.0)
                          );
        }

        mScale = mFromScale
            + (mToScale - mFromScale)
            * qBound<qreal>(0,
                            (mProgress - mScaleStartProgress)
                            / (mScaleEndProgress - mScaleStartProgress),
                            1.0);

        mOpacity = mFromOpacity
                   + (mToOpacity - mFromOpacity)
                   * qBound<qreal>(0,
                                   (mProgress - mOpacityStartProgress)
                                   / (mOpacityEndProgress - mOpacityStartProgress),
                                   1.0);
    }
    virtual void reset()
    {
        mProgress = 0;
        mOpacity = mFromOpacity;
        mScale = mScale;
        mPosition = mFromPosition;
    };
    //! reimp_end
};

#endif
