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

#ifndef BORDERPANRECOGNIZER_H
#define BORDERPANRECOGNIZER_H

#include <QGestureRecognizer>
#include <QTouchEvent>

class PanGesture;
class QGraphicsSceneMouseEvent;
class QGraphicsWidget;
class QPoint;

class BorderPanRecognizer : public QGestureRecognizer
{
public:
    //! \reimp
    virtual QGesture *create(QObject *target);
    virtual Result recognize(QGesture *gesture, QObject *watched, QEvent *event);
    virtual void reset(QGesture *gesture);
    //! \reimp_end

    static void registerSharedRecognizer();
    static Qt::GestureType sharedGestureType();
    static void unregisterSharedRecognizer();
    static BorderPanRecognizer *instance();

    void setTimeout(int timeoutInterval);
    void setStartThreshold(int threshold);
    void setFinishThreshold(int threshold);
    void setInitialMovement(int initialMovement);

    bool maybePanGesture() const;
private:
    //! Constructor
    BorderPanRecognizer();

    //! Destructor
    virtual ~BorderPanRecognizer();

    QGestureRecognizer::Result recognizeInit(PanGesture &gesture,
                                             const QGraphicsSceneMouseEvent *mouseEvent,
                                             const QGraphicsWidget *widget);
    QGestureRecognizer::Result recognizeUpdate(PanGesture &gesture,
                                               const QGraphicsSceneMouseEvent *mouseEvent,
                                               const QGraphicsWidget *widget);
    QGestureRecognizer::Result recognizeFinalize(PanGesture &gesture,
                                                 const QGraphicsSceneMouseEvent *mouseEvent,
                                                 const QGraphicsWidget *widget);

private:
    static BorderPanRecognizer *sharedInstance;
    static Qt::GestureType sharedType;

    int timeoutInterval;
    int startThreshold;
    int finishThreshold;
    int initialMovement;

    QTouchEvent lastTouchEvent; //!< Used for resending previous event. Used by TouchForwardFilter.
    bool mMaybePanGesture;
};

#endif // BORDERPANRECOGNIZER_H
